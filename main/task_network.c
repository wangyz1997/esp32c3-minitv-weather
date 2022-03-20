#include "task_network.h"
#include "task_ui.h"

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

    /* 显示正在连接屏幕 */
    task_ui_load_connecting_screen();
    task_ui_update_connecting_status(status_waiting_wifi);
    /* 初始化WiFi并等待连接 */
    app_wifi_init();
    xEventGroupWaitBits(wifi_event_group, WIFI_GOT_IP_EVT, pdTRUE, pdFALSE, portMAX_DELAY);
    /* 显示连接成功屏幕 */
    task_ui_update_connecting_status(status_fetching_data);
    // xEventGroupSetBits(network_event_group, NETWORK_CONNECTED_EVT);
    /* 初始化sntp与时区 */
    app_wifi_sntp_init();
    /* 获取天气 */
    int error = network_qweather_update_now();
    if(error != 200) { //获取失败
        xEventGroupSetBits(network_event_group, NETWORK_DATA_ERROR_EVT); //发送获取失败事件
    }
    /* 等待获取网络时间 */
    while(sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
        vTaskDelay(1);
    }
    /* 数据获取成功 更新数据并切换到主界面 */
    // xEventGroupSetBits(network_event_group, NETWORK_DATA_READY_EVT);
    task_ui_update_city_name(network_qweather_get_city_name());
    task_ui_update_weather_now(network_qweather_get_weather_now_info());
    task_ui_update_weather_forecast(network_qweather_get_weather_forecast_info());
    task_ui_load_main_screen();

    uint32_t cnt = 0;
    while(1) {
        cnt ++;

        if(gpio_get_level(9) == 0) {
            cnt += update_period_miniute*60*10;
        }
        
        if(cnt >= update_period_miniute*60*10) {
            error = network_qweather_update_now();
            if(error != 200) { //获取失败
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
