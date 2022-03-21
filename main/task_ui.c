#include "task_ui.h"
#include "task_network.h"

#include "lv_port_disp.h"
#include "lv_port_tick.h"
#include "lv_fonts.h"

static const char *TAG = "task_ui";
static QueueHandle_t task_ui_queue_handle;

typedef enum {
    /* 界面切换 */
    task_ui_message_load_connecting_screen,
    task_ui_message_load_main_screen,
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

void task_ui_load_connecting_screen()
{
    task_ui_message_t msg = {
        .msg_type = task_ui_message_load_connecting_screen,
        .msg_param = 0,
        .msg_payload = NULL
    };
    xQueueSend(task_ui_queue_handle, &msg, portMAX_DELAY);
}

void task_ui_load_main_screen()
{
    task_ui_message_t msg = {
        .msg_type = task_ui_message_load_main_screen,
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

    case task_ui_message_load_connecting_screen:
        ui_connecting_screen_load();
        break;
    case task_ui_message_load_main_screen:
        ui_main_screen_load();
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

    ui_connecting_screen_create();
    ui_main_screen_create();
}

static void task_handler(void *_args)
{
    task_ui_init();

    task_ui_queue_handle = xQueueCreate(4, sizeof(task_ui_message_t));
    task_network_create();

    while(1) {
        task_ui_message_t msg;
        while(xQueueReceive(task_ui_queue_handle, &msg, pdMS_TO_TICKS(20)) == pdPASS) {
            task_ui_event_handler(&msg);
        }

        ui_main_screen_timedate_update();

        lv_task_handler();
    }
}

void task_ui_create()
{
    xTaskCreate(task_handler, "UI Task", 4096, NULL, 4, NULL);
}
