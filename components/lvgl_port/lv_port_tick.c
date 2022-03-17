#include "lv_port_tick.h"

#include <esp_timer.h>

static void lv_tick_handler(void *arg)
{
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

void lv_create_tick()
{
    esp_timer_handle_t periodic_timer;

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_handler,
        .name = "lv_tick_handler",
    };

    ESP_ERROR_CHECK(
        esp_timer_create(&periodic_timer_args, &periodic_timer)
    );
    ESP_ERROR_CHECK(
        esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000)
    );
}