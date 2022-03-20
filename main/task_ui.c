#include "task_ui.h"
#include "task_network.h"

#include "lv_port_disp.h"
#include "lv_port_tick.h"
#include "lv_fonts.h"

static const char *TAG = "task_ui";
static QueueHandle_t task_ui_queue_handle;

typedef enum {
    task_ui_screen_none = 0,
    task_ui_screen_connecting = 1,
    task_ui_screen_main = 2,
} task_ui_screen_t;

typedef struct {
    void (*show_func)();
    void (*delete_func)();
} task_ui_screen_functions_t;

static task_ui_screen_t current_screen = task_ui_screen_none;

/* 各个屏幕的创建与删除函数 */
const task_ui_screen_functions_t task_ui_screen_functions[3] = {
    {NULL, NULL},
    {ui_connecting_screen_show, ui_connecting_screen_delete},
    {ui_main_screen_show, ui_main_screen_delete},
};

typedef enum {
    task_ui_message_none = 0,
    /* 界面切换 */
    task_ui_message_switch_connecting_screen = 1,
    task_ui_message_switch_main_screen = 2,
    /* 连接界面操作 */
    task_ui_message_update_connecting_status,
    /* 主界面操作 */
    task_ui_message_update_city_name,
    task_ui_message_update_weather_now,
    task_ui_message_update_weather_forecast,
    task_ui_message_update_wifi_status,
} task_ui_message_type_t;

typedef struct {
    task_ui_message_type_t msg_type;
    int msg_param;
    const void *msg_payload;
} task_ui_message_t;

void task_ui_switch_connecting_screen()
{
    task_ui_message_t msg = {
        .msg_type = task_ui_message_switch_connecting_screen,
        .msg_param = 0,
        .msg_payload = NULL
    };
    xQueueSend(task_ui_queue_handle, &msg, portMAX_DELAY);
}

void task_ui_switch_main_screen()
{
    task_ui_message_t msg = {
        .msg_type = task_ui_message_switch_main_screen,
        .msg_param = 0,
        .msg_payload = NULL
    };
    xQueueSend(task_ui_queue_handle, &msg, portMAX_DELAY);
}

void task_ui_update_connecting_status(ui_connecting_screen_status_t status)
{
    task_ui_message_t msg = {
        .msg_type = task_ui_message_update_connecting_status,
        .msg_param = status,
        .msg_payload = NULL
    };
    xQueueSend(task_ui_queue_handle, &msg, portMAX_DELAY);
}

void task_ui_update_city_name(const char *city_name)
{
    task_ui_message_t msg = {
        .msg_type = task_ui_message_update_city_name,
        .msg_param = 0,
        .msg_payload = city_name
    };
    xQueueSend(task_ui_queue_handle, &msg, portMAX_DELAY);
}

void task_ui_update_weather_now(weather_now_info_t *weather_now_info)
{
    task_ui_message_t msg = {
        .msg_type = task_ui_message_update_weather_now,
        .msg_param = 0,
        .msg_payload = weather_now_info
    };
    xQueueSend(task_ui_queue_handle, &msg, portMAX_DELAY);
}

void task_ui_update_weather_forecast(weather_forecast_info_t *weather_forecast_info)
{
    task_ui_message_t msg = {
        .msg_type = task_ui_message_update_weather_forecast,
        .msg_param = 0,
        .msg_payload = weather_forecast_info
    };
    xQueueSend(task_ui_queue_handle, &msg, portMAX_DELAY);
}

void task_ui_event_handler(task_ui_message_t *msg)
{
    switch(msg->msg_type) {

    case task_ui_message_switch_connecting_screen:
    case task_ui_message_switch_main_screen:
        if((task_ui_screen_t)msg->msg_type != current_screen) {
            if(current_screen != task_ui_screen_none) {
                task_ui_screen_functions[current_screen].delete_func();
            }
            current_screen = msg->msg_type;
            task_ui_screen_functions[current_screen].show_func();
        }
        break;

    case task_ui_message_update_connecting_status:
        ui_connecting_screen_update(msg->msg_param);
        break;

    case task_ui_message_update_city_name:
        ui_main_screen_city_update(msg->msg_payload);
        break;
    case task_ui_message_update_weather_now:
        ui_main_screen_weather_now_update(msg->msg_payload);
        break;
    case task_ui_message_update_weather_forecast:
        ui_main_screen_weather_forecast_update(msg->msg_payload);
        break;
    default:
        ESP_LOGW(TAG, "unhandeled event, type=%d", msg->msg_type);
        break;
    }
}

void task_ui_init()
{
	lv_init();
    lv_port_disp_init();
    lv_create_tick();
    
    lv_port_font_harmony_sans_20_load("font_hs20");
}

static void task_handler(void *_args)
{
    task_ui_init();
    task_network_create();

    task_ui_queue_handle = xQueueCreate(8, sizeof(task_ui_message_t));

    while(1) {
        task_ui_message_t msg;
        while(xQueueReceive(task_ui_queue_handle, &msg, pdMS_TO_TICKS(20)) == pdPASS) {
            task_ui_event_handler(&msg);
        }

        if(current_screen == task_ui_screen_main) {
            ui_main_screen_timedate_update();
        }

        lv_task_handler();
    }
}

void task_ui_create()
{
    xTaskCreate(task_handler, "UI Task", 4096, NULL, 4, NULL);
}
