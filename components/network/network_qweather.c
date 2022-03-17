#include "network_qweather.h"
#include "network_request.h"

#include <cJSON.h>

#include "sdkconfig.h"

static const char *TAG = "network_qweather";

static const char *qweather_geo_api_host = "geoapi.qweather.com";
static const char *qweather_city_lookup_url = "/v2/city/lookup?location=%s&adm=%s&key=%s";

static const char *qweather_weather_api_host = "devapi.qweather.com";
static const char *qweather_weather_now_url = "/v7/weather/now?location=%d&key=%s";
static const char *qweather_weather_forecast_url = "/v7/weather/3d?location=%d&key=%s";

static uint32_t qweather_city_id = 0; //城市ID
static char qweather_city_name[32] = {0}; //城市名
static weather_now_info_t weather_now_info;
static weather_forecast_info_t weather_forecast_info;

static esp_err_t network_qweather_get_city_id_name()
{
    char url[80];
    uint8_t error = 0;
    esp_err_t result = ESP_OK;

    /* 构造url */
    sprintf(url, qweather_city_lookup_url, CONFIG_APP_QWEATHER_DISTRICT, CONFIG_APP_QWEATHER_ADM, CONFIG_APP_QWEATHER_KEY);
    const size_t resp_buf_size = 2048;
    char *response = malloc(resp_buf_size);
    size_t resp_size = network_https_request(response, resp_buf_size, qweather_geo_api_host, url, AAA_CA_CERT);
    
    if(resp_size) { //返回响应
        cJSON *root = cJSON_Parse(response);
        if(cJSON_IsInvalid(root)) {
            error ++;
        }
        /* 城市列表 */
        cJSON *location = NULL;
        if(error == 0) {
            location = cJSON_GetObjectItem(root, "location");
            if(cJSON_IsInvalid(location)) {
                error ++;
            }
        }
        /* 列表中的第一个城市 */
        cJSON *city = NULL;
        if(error == 0) {
            city = cJSON_GetArrayItem(location, 0);
            if(cJSON_IsInvalid(city)) {
                error ++;
            }
        }
        /* 城市名 */
        cJSON *city_name = NULL;
        if(error == 0) {
            city_name = cJSON_GetObjectItem(city, "name");
            if(cJSON_IsInvalid(city_name)) {
                error ++;
            } else {
                strcpy(qweather_city_name, cJSON_GetStringValue(city_name));
            }
        }
        /* 城市ID */
        cJSON *city_id = NULL;
        if(error == 0) {
            city_id = cJSON_GetObjectItem(city, "id");
            if(cJSON_IsInvalid(city_id)) {
                error ++;
            } else {
                qweather_city_id = atoi(cJSON_GetStringValue(city_id));
            }
        }
        
        if(error) { //响应代码无效
            ESP_LOGE(TAG, "json parse error while getting city info from qweather, error=%d", error);
            result = ESP_FAIL;
        } else {
            ESP_LOGI(TAG, "got city id: %d(%s)", qweather_city_id, qweather_city_name);
        }
        cJSON_Delete(root);
    } else { //未返回响应
        ESP_LOGE(TAG, "http response error while getting city info from qweather");
        result = ESP_FAIL;
    }

    free(response);
    return result;
}

static esp_err_t network_qweather_update_weather_now()
{
    char url[80];
    uint8_t error = 0;
    esp_err_t result = ESP_OK;

    /* 构造url */
    sprintf(url, qweather_weather_now_url, qweather_city_id, CONFIG_APP_QWEATHER_KEY);
    const size_t resp_buf_size = 2048;
    char *response = malloc(resp_buf_size);
    size_t resp_size = network_https_request(response, resp_buf_size, qweather_weather_api_host, url, AAA_CA_CERT);
    
    if(resp_size) { //返回响应
        cJSON *root = cJSON_Parse(response);
        if(cJSON_IsInvalid(root)) {
            error ++;
        }
        /* 天气信息 */
        cJSON *weather_now = NULL;
        if(error == 0) {
            weather_now = cJSON_GetObjectItem(root, "now");
            if(cJSON_IsInvalid(weather_now)) {
                error ++;
            }
        }
        /* 温度 */
        cJSON *temp = NULL;
        if(error == 0) {
            temp = cJSON_GetObjectItem(weather_now, "temp");
            if(cJSON_IsInvalid(temp)) {
                error ++;
            } else {
                weather_now_info.temperature = atoi(cJSON_GetStringValue(temp));
            }
        }
        /* 湿度 */
        cJSON *humidity = NULL;
        if(error == 0) {
            humidity = cJSON_GetObjectItem(weather_now, "humidity");
            if(cJSON_IsInvalid(humidity)) {
                error ++;
            } else {
                weather_now_info.humidity = atoi(cJSON_GetStringValue(humidity));
            }
        }
        /* 体感温度 */
        cJSON *feelslike = NULL;
        if(error == 0) {
            feelslike = cJSON_GetObjectItem(weather_now, "feelsLike");
            if(cJSON_IsInvalid(feelslike)) {
                error ++;
            } else {
                weather_now_info.feels_like = atoi(cJSON_GetStringValue(feelslike));
            }
        }
        /* 天气图标 */
        cJSON *icon = NULL;
        if(error == 0) {
            icon = cJSON_GetObjectItem(weather_now, "icon");
            if(cJSON_IsInvalid(icon)) {
                error ++;
            } else {
                weather_now_info.weather_icon = atoi(cJSON_GetStringValue(icon));
            }
        }
        /* 天气 */
        cJSON *weather = NULL;
        if(error == 0) {
            weather = cJSON_GetObjectItem(weather_now, "text");
            if(cJSON_IsInvalid(weather)) {
                error ++;
            } else {
                strcpy(weather_now_info.weather, cJSON_GetStringValue(weather));
            }
        }
        
        if(error) { //响应代码无效
            ESP_LOGE(TAG, "json parse error while getting city info from qweather, error=%d", error);
            result = ESP_FAIL;
        } else {
            ESP_LOGI(TAG, "%s: %s, %d℃, %d%%RH, feels like %d℃", qweather_city_name, weather_now_info.weather,
                weather_now_info.temperature, weather_now_info.humidity, weather_now_info.feels_like);
        }
        cJSON_Delete(root);
    } else { //未返回响应
        ESP_LOGE(TAG, "http response error while getting city info from qweather");
        result = ESP_FAIL;
    }

    free(response);
    return result;
}

static esp_err_t network_qweather_update_weather_forecast()
{
    char url[80];
    uint8_t error = 0;
    esp_err_t result = ESP_OK;

    /* 构造url */
    sprintf(url, qweather_weather_forecast_url, qweather_city_id, CONFIG_APP_QWEATHER_KEY);
    const size_t resp_buf_size = 2048;
    char *response = malloc(resp_buf_size);
    size_t resp_size = network_https_request(response, resp_buf_size, qweather_weather_api_host, url, AAA_CA_CERT);
    
    if(resp_size) { //返回响应
        cJSON *root = cJSON_Parse(response);
        if(cJSON_IsInvalid(root)) {
            error ++;
        }
        /* 天气预报信息 */
        cJSON *weather_days = NULL;
        if(error == 0) {
            weather_days = cJSON_GetObjectItem(root, "daily");
            if(cJSON_IsInvalid(weather_days)) {
                error ++;
            }
        }
        /* 第一天 */
        cJSON *weather_day1 = NULL;
        if(error == 0) {
            weather_day1 = cJSON_GetArrayItem(weather_days, 0);
            if(cJSON_IsInvalid(weather_day1)) {
                error ++;
            }
        }
        /* 第一天最高温度 */
        cJSON *day1_high_temp = NULL;
        if(error == 0) {
            day1_high_temp = cJSON_GetObjectItem(weather_day1, "tempMax");
            if(cJSON_IsInvalid(day1_high_temp)) {
                error ++;
            } else {
                weather_forecast_info.day1_high_temp = atoi(cJSON_GetStringValue(day1_high_temp));
            }
        }
        /* 第一天最低温度 */
        cJSON *day1_low_temp = NULL;
        if(error == 0) {
            day1_low_temp = cJSON_GetObjectItem(weather_day1, "tempMin");
            if(cJSON_IsInvalid(day1_low_temp)) {
                error ++;
            } else {
                weather_forecast_info.day1_low_temp = atoi(cJSON_GetStringValue(day1_low_temp));
            }
        }
        /* 第一天天气 */
        cJSON *day1_weather = NULL;
        if(error == 0) {
            day1_weather = cJSON_GetObjectItem(weather_day1, "textDay");
            if(cJSON_IsInvalid(day1_weather)) {
                error ++;
            } else {
                strcpy(weather_forecast_info.day1_weather, cJSON_GetStringValue(day1_weather));
            }
        }
        /* 第二天 */
        cJSON *weather_day2 = NULL;
        if(error == 0) {
            weather_day2 = cJSON_GetArrayItem(weather_days, 1);
            if(cJSON_IsInvalid(weather_day2)) {
                error ++;
            }
        }
        /* 第二天最高温度 */
        cJSON *day2_high_temp = NULL;
        if(error == 0) {
            day2_high_temp = cJSON_GetObjectItem(weather_day2, "tempMax");
            if(cJSON_IsInvalid(day2_high_temp)) {
                error ++;
            } else {
                weather_forecast_info.day2_high_temp = atoi(cJSON_GetStringValue(day2_high_temp));
            }
        }
        /* 第二天最低温度 */
        cJSON *day2_low_temp = NULL;
        if(error == 0) {
            day2_low_temp = cJSON_GetObjectItem(weather_day2, "tempMin");
            if(cJSON_IsInvalid(day2_low_temp)) {
                error ++;
            } else {
                weather_forecast_info.day2_low_temp = atoi(cJSON_GetStringValue(day2_low_temp));
            }
        }
        /* 第二天天气 */
        cJSON *day2_weather = NULL;
        if(error == 0) {
            day2_weather = cJSON_GetObjectItem(weather_day2, "textDay");
            if(cJSON_IsInvalid(day2_weather)) {
                error ++;
            } else {
                strcpy(weather_forecast_info.day2_weather, cJSON_GetStringValue(day2_weather));
            }
        }
        
        if(error) { //响应代码无效
            ESP_LOGE(TAG, "json parse error while getting city info from qweather, error=%d", error);
            result = ESP_FAIL;
        } else {
            ESP_LOGI(TAG, "forecast: %s, %d~%d℃; %s, %d~%d℃",
                weather_forecast_info.day1_weather, weather_forecast_info.day1_low_temp, weather_forecast_info.day1_high_temp,
                weather_forecast_info.day2_weather, weather_forecast_info.day2_low_temp, weather_forecast_info.day2_high_temp
            );
        }
        cJSON_Delete(root);
    } else { //未返回响应
        ESP_LOGE(TAG, "http response error while getting city info from qweather");
        result = ESP_FAIL;
    }

    free(response);
    return result;
}

esp_err_t network_qweather_update_now()
{
    esp_err_t error;
    if(qweather_city_id == 0) { //还未获取城市信息
        ESP_LOGI(TAG, "getting city info...");
        error = network_qweather_get_city_id_name();
        
        if(error != ESP_OK) {
            return error;
        }
    }
    
    error = network_qweather_update_weather_now();
    if(error != ESP_OK) {
        return error;
    }

    return network_qweather_update_weather_forecast();
}

char *network_qweather_get_city_name()
{
    return qweather_city_name;
}

weather_now_info_t *network_qweather_get_weather_now_info()
{
    return &weather_now_info;
}

weather_forecast_info_t *network_qweather_get_weather_forecast_info()
{
    return &weather_forecast_info;
}
