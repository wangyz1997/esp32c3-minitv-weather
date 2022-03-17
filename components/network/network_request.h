#ifndef __NETWORK_REQUEST_H
#define __NETWORK_REQUEST_H

#include <esp_system.h>
#include <esp_tls.h>

extern const uint8_t digicert_root_ca_start[]   asm("_binary_digicert_pem_start");
extern const uint8_t digicert_root_ca_end[]     asm("_binary_digicert_pem_end");
extern const uint8_t aaa_root_ca_start[]        asm("_binary_aaa_pem_start");
extern const uint8_t aaa_root_ca_end[]          asm("_binary_aaa_pem_end");

#define DIGICERT_CA_CERT digicert_root_ca_start,digicert_root_ca_end
#define AAA_CA_CERT aaa_root_ca_start,aaa_root_ca_end

size_t network_https_request(void *out_buf, size_t out_buf_size, const char *host, const char *url, const void *ca_cert_begin, const void* ca_cert_end);

#endif
