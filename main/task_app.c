#include "task_app.h"
#include "task_lvgl.h"
#include "task_network.h"

#include <esp_err.h>
#include <esp_log.h>

#include "lv_port_disp.h"
#include "lv_port_tick.h"
#include "lv_fonts.h"

#include "ui_connecting_screen.h"
#include "ui_main_screen.h"

#include "network_qweather.h"

static void task_handler(void *_args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(1000);

    /* 显示正在连接屏幕 */
    task_lvgl_semphr_take();
    ui_connecting_screen_show(status_waiting_wifi);
    task_lvgl_semphr_give();
    /* 等待连接 */
    xEventGroupWaitBits(network_event_group, NETWORK_CONNECTED_EVT, pdTRUE, pdFALSE, portMAX_DELAY);
    /* 更新显示 */
    task_lvgl_semphr_take();
    ui_connecting_screen_update(status_fetching_data);
    task_lvgl_semphr_give();
    /* 等待数据获取 */
    EventBits_t ev_bits;
    while(1) {
        ev_bits = xEventGroupWaitBits(network_event_group, NETWORK_DATA_READY_EVT|NETWORK_DATA_ERROR_EVT, pdTRUE, pdFALSE, portMAX_DELAY);
        if(ev_bits & NETWORK_DATA_ERROR_EVT) {
            task_lvgl_semphr_take();
            ui_connecting_screen_update(status_data_error);
            task_lvgl_semphr_give();
            while(1) {
                vTaskDelay(1); //程序在此处停止
            }
        } else if(ev_bits & NETWORK_DATA_READY_EVT) {
            break;
        }
    }
    /* 数据获取成功，显示主界面并更新内容 */
    task_lvgl_semphr_take();
    ui_connecting_screen_delete();
    ui_main_screen_show();
    ui_main_screen_timedate_update();
    ui_main_screen_city_update(network_qweather_get_city_name());
    ui_main_screen_weather_now_update(network_qweather_get_weather_now_info());
    ui_main_screen_weather_forecast_update(network_qweather_get_weather_forecast_info());
    task_lvgl_semphr_give();

    while(1) {
        task_lvgl_semphr_take();
        ui_main_screen_timedate_update();
        task_lvgl_semphr_give();

        ev_bits = xEventGroupWaitBits(network_event_group, NETWORK_DATA_READY_EVT|NETWORK_DATA_ERROR_EVT, pdTRUE, pdFALSE, 0);

        if(ev_bits & NETWORK_DATA_READY_EVT) {
            task_lvgl_semphr_take();
            ui_main_screen_weather_now_update(network_qweather_get_weather_now_info());
            task_lvgl_semphr_give();
        }

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

void task_app_create()
{
    xTaskCreate(task_handler, "App Task", 4096, NULL, 4, NULL);
}
