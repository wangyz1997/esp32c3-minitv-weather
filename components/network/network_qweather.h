#ifndef __NETWORK_QWEATHER_H
#define __NETWORK_QWEATHER_H

#include <esp_err.h>
#include <esp_log.h>

typedef struct {
    int16_t temperature; //温度
    uint16_t humidity; //湿度
    int16_t feels_like; //体感温度
    uint16_t weather_icon; //天气图标
    char weather[32]; //天气
} weather_now_info_t;

typedef struct {
    int16_t day1_high_temp; //最高温度
    int16_t day1_low_temp; //最低湿度
    char day1_weather[32]; //天气
    int16_t day2_high_temp; //最高温度
    int16_t day2_low_temp; //最低湿度
    char day2_weather[32]; //天气
} weather_forecast_info_t;

int network_qweather_update_now();
char *network_qweather_get_city_name();
weather_now_info_t *network_qweather_get_weather_now_info();
weather_forecast_info_t *network_qweather_get_weather_forecast_info();

#endif
