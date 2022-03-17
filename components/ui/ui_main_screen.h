#ifndef __UI_MAIN_SCREEN_H
#define __UI_MAIN_SCREEN_H

#include "network_qweather.h"

void ui_main_screen_show();
void ui_main_screen_timedate_update();
void ui_main_screen_weather_now_update(weather_now_info_t *weather_now_info);
void ui_main_screen_weather_forecast_update(weather_forecast_info_t *weather_forecast_info);
void ui_main_screen_city_update(const char *city);
void ui_main_screen_delete();

#endif
