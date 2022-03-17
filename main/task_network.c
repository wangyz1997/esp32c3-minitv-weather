#include "task_network.h"

#include <esp_sntp.h>
#include <driver/gpio.h>

#include "network_wifi.h"
#include "network_request.h"
#include "network_qweather.h"

const uint32_t update_period_miniute = 10;

EventGroupHandle_t network_event_group;

static void task_handler(void *_args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(100);

    /* 初始化WiFi并等待连接 */
    app_wifi_init();
    xEventGroupWaitBits(wifi_event_group, WIFI_GOT_IP_EVT, pdTRUE, pdFALSE, portMAX_DELAY);
    /* 发送连接成功事件 */
    xEventGroupSetBits(network_event_group, NETWORK_CONNECTED_EVT);
    
    /* 初始化sntp与时区 */
    app_wifi_sntp_init();
    
    /* 获取天气 */
    esp_err_t error = network_qweather_update_now();
    if(error != ESP_OK) { //获取失败
        xEventGroupSetBits(network_event_group, NETWORK_DATA_ERROR_EVT); //发送获取失败事件
    }
    /* 等待获取网络时间 */
    while(sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
        vTaskDelay(1);
    }
    /* 发送获取数据成功事件 */
    xEventGroupSetBits(network_event_group, NETWORK_DATA_READY_EVT);

    uint32_t cnt = 0;
    while(1) {
        cnt ++;

        if(gpio_get_level(9) == 0) {
            cnt += update_period_miniute*60*10;
        }
        
        if(cnt >= update_period_miniute*60*10) {
            error = network_qweather_update_now();
            if(error != ESP_OK) { //获取失败
                xEventGroupSetBits(network_event_group, NETWORK_DATA_ERROR_EVT); //发送获取失败事件
            } else {
                xEventGroupSetBits(network_event_group, NETWORK_DATA_READY_EVT); //发送获取成功事件
            }

            cnt = 0;
        }

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

void task_network_create()
{
    network_event_group = xEventGroupCreate();
    xTaskCreate(task_handler, "Network Task", 8192, NULL, 3, NULL);
}
