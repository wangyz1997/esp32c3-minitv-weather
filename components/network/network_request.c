#include "network_request.h"
#include "network_utils.h"

#include <esp_err.h>
#include <esp_log.h>

#include "zlib.h"
#include "zutil.h"
#include "inftrees.h"
#include "inflate.h"

const static char *TAG = "network_requests";

static const char HTTPS_REQUEST[] = "GET %s HTTP/1.1\r\n"
                                    "Host: %s\r\n"
                                    "User-Agent: ESP32 HTTP Client/1.0\r\n"
                                    "\r\n";

static size_t network_tls_transfer(esp_tls_cfg_t *cfg, void *out_buf, size_t out_buf_size, const char *host, const char *url)
{
    size_t out_size = 0;
    /* 建立新的TLS连接 */
    struct esp_tls *tls = esp_tls_init();
    if (tls == NULL) {
        ESP_LOGE(TAG, "error while initialize tls connection");
        goto exit;
    }

    if(esp_tls_conn_new_sync(host, strlen(host), 443, cfg, tls) == 0) {
        ESP_LOGE(TAG, "error while creating tls connection");
        goto exit;
    }
    ESP_LOGI(TAG, "connected to %s, sending request...", host);
    
    /* 构造http请求 */
    int request_len = snprintf(out_buf, out_buf_size, HTTPS_REQUEST, url, host) + 1;
    /* 通过TLS连接发送数据 */
    size_t written = 0;
    do {
        ssize_t ret = esp_tls_conn_write(tls, out_buf + written, request_len - written); //发送数据
        if (ret >= 0) { //发送成功 返回实际写入的数据大小
            written += ret;
        } else if (ret != ESP_TLS_ERR_SSL_WANT_READ && ret != ESP_TLS_ERR_SSL_WANT_WRITE) { //发送失败
            ESP_LOGE(TAG, "error while sending request");
            goto exit;
        }
    } while(written < request_len);

    ESP_LOGI(TAG, "request sent, reading response...");
    /* 通过TLS连接接收数据 */
    size_t remaining = out_buf_size - 1; //缓冲区剩余大小
    do {
        ssize_t ret = esp_tls_conn_read(tls, out_buf+(out_buf_size-remaining-1), remaining); //接收数据
        if(ret == ESP_TLS_ERR_SSL_WANT_WRITE || ret == ESP_TLS_ERR_SSL_WANT_READ) {
            continue;
        } else if (ret < 0) { //接收错误
            ESP_LOGE(TAG, "error while reading request");
            break;
        } else if (ret == 0) { //接收完毕
            out_size = out_buf_size - remaining - 1;
            ESP_LOGI(TAG, "connection closed, %d bytes read", out_size);
            break;
        }
        remaining -= ret; //剩余缓冲区空间减去本次接收到的大小
    } while(1);

exit:
    esp_tls_conn_delete(tls);
    ((char*)out_buf)[out_size] = '\0';
    return out_size;
}

static int network_gzip_decompress(void *in_buf, size_t in_size, void *out_buf, size_t *out_size, size_t out_buf_size)
{
    int err = 0;
    z_stream d_stream = {0}; /* decompression stream */
    d_stream.zalloc = NULL;
    d_stream.zfree = NULL;
    d_stream.opaque = NULL;
    d_stream.next_in  = in_buf;
    d_stream.avail_in = 0;
    d_stream.next_out = out_buf;

    if((err=inflateInit2(&d_stream, 47)) != Z_OK) {
        return err;
    }
    while(d_stream.total_out < out_buf_size-1 && d_stream.total_in < in_size) {
        d_stream.avail_in = d_stream.avail_out = 1;
        if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) {
            break;
        }
        if(err != Z_OK) {
            return err;
        }
    }

    if((err=inflateEnd(&d_stream)) != Z_OK) {
        return err;
    }

    *out_size = d_stream.total_out;
    ((char*)out_buf)[*out_size] = '\0';

    return Z_OK;
}

size_t network_https_request(void *out_buf, size_t out_buf_size, const char *host, const char *url, const void *ca_cert_begin, const void* ca_cert_end)
{
    size_t out_size = 0;

    esp_tls_cfg_t tls_cfg_ca_cert = {
        .cacert_buf = ca_cert_begin,
        .cacert_bytes = ca_cert_end - ca_cert_begin,
    };

    /* 申请内存 */
    void *http_response = malloc(out_buf_size);
    if(http_response == NULL) {
        ESP_LOGE(TAG, "error while allocting memory for https request");
        out_size = -1;
        goto exit;
    }
    /* 发送https请求 */
    size_t http_resp_size = network_tls_transfer(&tls_cfg_ca_cert, http_response, out_buf_size, host, url);
    if(http_resp_size == 0) {
        out_size = -1;
        goto exit;
    }

    /* 取得响应的长度与起始地址 */
    void *resp_body = strstr(http_response, "\r\n\r\n")+strlen("\r\n\r\n");
    size_t resp_length;
    char *length_string = strcasestr(http_response, "Content-Length:");
    if(length_string) {
        sscanf(length_string, "%*s%d", &resp_length);
    } else {
        ESP_LOGE(TAG, "unsupported chunked transfer encoding");
        goto exit;
    }

    /* 检查响应内容是否被gzip压缩 */
    uint8_t gzip_encoded = 0;
    char cotent_encoding_string[16];
    char *content_encoding_line = strcasestr(http_response, "Content-Encoding:");
    if(content_encoding_line) {
        sscanf(content_encoding_line, "%*s%s", cotent_encoding_string);
        if(strcasestr(cotent_encoding_string, "gzip")) {
            gzip_encoded = 1;
        }
    }
    /* 判断内容是否过长 */
    ESP_LOGI(TAG, "https response length: %d bytes", resp_length);
    if(resp_length > out_buf_size-(resp_body-http_response)) {
        resp_length = out_buf_size-(resp_body-http_response);
        ESP_LOGW(TAG, "response too long, shrinking to %d bytes", resp_length);
        if(gzip_encoded) {
            ESP_LOGE(TAG, "gzip decode is not possible on shrinked buffer");
            goto exit;
        }
    }

    if(gzip_encoded) { //gzip压缩后的响应内容
        ESP_LOGD(TAG, "gzip encoded response, decompressing...");
        /* 解压请求内容 */
        int ret = network_gzip_decompress(resp_body, resp_length, out_buf, &out_size, out_buf_size);
        if(ret != ESP_OK) {
            out_size = -1;
            ESP_LOGE(TAG, "gzip data decompression failed, code=%d", ret);
        }
        ESP_LOGD(TAG, "response size after decompression: %d bytes", out_size);
    } else { //无压缩的响应内容
        memcpy(out_buf, resp_body, resp_length);
        out_size = resp_length;
    }

exit:
    free(http_response);
    return out_size;
}
