#ifndef __NETWORK_WIFI_H
#define __NETWORK_WIFI_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#define WIFI_CONNECTED_EVT     BIT0
#define WIFI_DISCONNECTED_EVT  BIT1
#define WIFI_GOT_IP_EVT        BIT2

extern EventGroupHandle_t wifi_event_group;

void app_wifi_init();
void app_wifi_sntp_init();

#endif
