#ifndef __TASK_UI_H
#define __TASK_UI_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "network_qweather.h"

#include "ui_main_screen.h"
#include "ui_connecting_screen.h"

void task_ui_create();

void task_ui_load_connecting_screen();
void task_ui_load_main_screen();

void task_ui_update_connecting_status(ui_connecting_screen_status_t status);

void task_ui_update_city_name(const char *city_name);
void task_ui_update_weather_now(weather_now_info_t *weather_now_info);
void task_ui_update_weather_forecast(weather_forecast_info_t *weather_forecast_info);

#endif
