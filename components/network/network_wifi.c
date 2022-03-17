#include "network_wifi.h"

#include <esp_log.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <nvs_flash.h>
#include <sntp/sntp.h>

#include <sdkconfig.h>

static const char *TAG = "network_wifi";
static esp_netif_t *wifi_netif;

EventGroupHandle_t wifi_event_group;

static void wifi_event_handler(int32_t event_id, void* event_data)
{
    switch(event_id)
    {
    case WIFI_EVENT_STA_START: //WiFi Station启动事件
        ESP_LOGI(TAG, "connecting wifi...");
        esp_wifi_connect(); //开始连接WiFi
        break;
    case WIFI_EVENT_STA_CONNECTED: //WiFi连接事件
        ESP_LOGI(TAG, "wifi connected");
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVT);
        break;
    case WIFI_EVENT_STA_DISCONNECTED: //WiFi Station断开事件
        esp_wifi_connect(); //尝试重新连接
        ESP_LOGW(TAG, "trying to reconnect wifi...");
        xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECTED_EVT);
        break;

    default:
        ESP_LOGW(TAG, "unhandled wifi event, id=%d", event_id);
    }
}

static void ip_event_handler(int32_t event_id, void* event_data)
{
    switch(event_id)
    {
    case IP_EVENT_STA_GOT_IP: //获取IP事件
        ESP_LOGI(TAG, "wifi got ip");
        xEventGroupSetBits(wifi_event_group, WIFI_GOT_IP_EVT);
        break;

    default:
        ESP_LOGW(TAG, "unhandled ip event, id=%d", event_id);
        break;
    }
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT) {
        wifi_event_handler(event_id, event_data);
    } else if(event_base == IP_EVENT) {
        ip_event_handler(event_id, event_data);
    } else {
        ESP_LOGW(TAG, "unhandled event: %s", event_base);
    }
}

void app_wifi_init()
{
    /* 初始化NVS */
    ESP_ERROR_CHECK(nvs_flash_init());
    /* 初始化消息循环并创建事件组 */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();
    /* 初始化底层IP协议栈 */
    ESP_ERROR_CHECK(esp_netif_init());
    /* 创建默认的WiFi Station接口 */
    wifi_netif = esp_netif_create_default_wifi_sta();
    /* 使用默认参数初始化WiFi */
    const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    /* 注册回调函数 */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    /* 配置WiFi */
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_APP_WIFI_SSID,
            .password = CONFIG_APP_WIFI_PASSWORD,
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = -127,
	        .threshold.authmode = WIFI_AUTH_OPEN,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    /* 启动并连接WiFi */
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_wifi_sntp_init()
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "ntp.aliyun.com");
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    sntp_init();
    /* 设置时区 */
    setenv("TZ", "CST-8", 1);
    tzset();
}
