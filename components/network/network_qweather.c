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

const char *network_qweather_get_error_text(int code)
{
    switch(code) {
    case 200:
        return "请求成功";
    case 204:
        return "请求成功，但你查询的地区暂时没有你需要的数据";
    case 400:
        return "请求错误，可能包含错误的请求参数或缺少必选的请求参数";
    case 401:
        return "认证失败，可能使用了错误的KEY、数字签名错误、KEY的类型错误（如使用SDK的KEY去访问Web API）";
    case 402:
        return "超过访问次数或余额不足以支持继续访问服务，你可以充值、升级访问量或等待访问量重置";
    case 403:
        return "无访问权限，可能是绑定的PackageName、BundleID、域名IP地址不一致，或者是需要额外付费的数据";
    case 404:
        return "查询的数据或地区不存在";
    case 429:
        return "超过限定的QPM（每分钟访问次数），请参考QPM说明";
    case 500:
        return "无响应或超时，接口服务异常请联系我们";
    case 600:
        return "JSON解析错误";
    default:
        return "";
    }
}

static int network_qweather_get_city_id_name()
{
    char url[100];
    int code = 200;

    /* 构造url */
    sprintf(url, qweather_city_lookup_url, CONFIG_APP_QWEATHER_DISTRICT, CONFIG_APP_QWEATHER_ADM, CONFIG_APP_QWEATHER_KEY);
    const size_t resp_buf_size = 2048;
    char *response = malloc(resp_buf_size);
    size_t resp_size = network_https_request(response, resp_buf_size, qweather_geo_api_host, url, AAA_CA_CERT);

    if(resp_size) { //返回响应
        cJSON *root = cJSON_Parse(response);
        if(root == NULL) {
            code = 600;
        }
        /* 响应代码 */
        cJSON *resp_code = NULL;
        if(code == 200) {
            resp_code = cJSON_GetObjectItem(root, "code");
            if(resp_code == NULL) {
                code = 600;
            } else {
                code = atoi(cJSON_GetStringValue(resp_code));
            }
        }
        /* 城市列表 */
        cJSON *location = NULL;
        if(code == 200) {
            location = cJSON_GetObjectItem(root, "location");
            if(location == NULL) {
                code = 600;
            }
        }
        /* 列表中的第一个城市 */
        cJSON *city = NULL;
        if(code == 200) {
            city = cJSON_GetArrayItem(location, 0);
            if(city == NULL) {
                code = 600;
            }
        }
        /* 城市名 */
        cJSON *city_name = NULL;
        if(code == 200) {
            city_name = cJSON_GetObjectItem(city, "name");
            if(city_name == NULL) {
                code = 600;
            } else {
                strcpy(qweather_city_name, cJSON_GetStringValue(city_name));
            }
        }
        /* 城市ID */
        cJSON *city_id = NULL;
        if(code == 200) {
            city_id = cJSON_GetObjectItem(city, "id");
            if(city_id == NULL) {
                code = 600;
            } else {
                qweather_city_id = atoi(cJSON_GetStringValue(city_id));
            }
        }
        cJSON_Delete(root);
    } else { //未返回响应
        code = 500; //无响应或超时
    }

    if(code == 200) {
        ESP_LOGI(TAG, "got city id: %d(%s)", qweather_city_id, qweather_city_name);
    } else { //响应代码无效
        ESP_LOGE(TAG, "error text: %s", network_qweather_get_error_text(code));
    }

    free(response);
    return code;
}

static int network_qweather_update_weather_now()
{
    char url[80];
    int code = 200;

    /* 构造url */
    sprintf(url, qweather_weather_now_url, qweather_city_id, CONFIG_APP_QWEATHER_KEY);
    const size_t resp_buf_size = 2048;
    char *response = malloc(resp_buf_size);
    size_t resp_size = network_https_request(response, resp_buf_size, qweather_weather_api_host, url, AAA_CA_CERT);

    if(resp_size) { //返回响应
        cJSON *root = cJSON_Parse(response);
        if(root == NULL) {
            code = 600;
        }
        /* 响应代码 */
        cJSON *resp_code = NULL;
        if(code == 200) {
            resp_code = cJSON_GetObjectItem(root, "code");
            if(resp_code == NULL) {
                code = 600;
            } else {
                code = atoi(cJSON_GetStringValue(resp_code));
            }
        }
        /* 天气信息 */
        cJSON *weather_now = NULL;
        if(code == 200) {
            weather_now = cJSON_GetObjectItem(root, "now");
            if(weather_now == NULL) {
                code = 600;
            }
        }
        /* 温度 */
        cJSON *temp = NULL;
        if(code == 200) {
            temp = cJSON_GetObjectItem(weather_now, "temp");
            if(temp == NULL) {
                code = 600;
            } else {
                weather_now_info.temperature = atoi(cJSON_GetStringValue(temp));
            }
        }
        /* 湿度 */
        cJSON *humidity = NULL;
        if(code == 200) {
            humidity = cJSON_GetObjectItem(weather_now, "humidity");
            if(humidity == NULL) {
                code = 600;
            } else {
                weather_now_info.humidity = atoi(cJSON_GetStringValue(humidity));
            }
        }
        /* 体感温度 */
        cJSON *feelslike = NULL;
        if(code == 200) {
            feelslike = cJSON_GetObjectItem(weather_now, "feelsLike");
            if(feelslike == NULL) {
                code = 600;
            } else {
                weather_now_info.feels_like = atoi(cJSON_GetStringValue(feelslike));
            }
        }
        /* 天气图标 */
        cJSON *icon = NULL;
        if(code == 200) {
            icon = cJSON_GetObjectItem(weather_now, "icon");
            if(icon == NULL) {
                code = 600;
            } else {
                weather_now_info.weather_icon = atoi(cJSON_GetStringValue(icon));
            }
        }
        /* 天气 */
        cJSON *weather = NULL;
        if(code == 200) {
            weather = cJSON_GetObjectItem(weather_now, "text");
            if(weather == NULL) {
                code = 600;
            } else {
                strcpy(weather_now_info.weather, cJSON_GetStringValue(weather));
            }
        }
        
        cJSON_Delete(root);
    } else { //未返回响应
        code = 500; //无响应或超时
    }

    if(code == 200) {
        ESP_LOGI(TAG, "%s: %s, %d℃, %d%%RH, feels like %d℃", qweather_city_name, weather_now_info.weather,
            weather_now_info.temperature, weather_now_info.humidity, weather_now_info.feels_like);
    } else { //响应代码无效
        ESP_LOGE(TAG, "error text: %s", network_qweather_get_error_text(code));
    }

    free(response);
    return code;
}

static int network_qweather_update_weather_forecast()
{
    char url[80];
    int code = 200;

    /* 构造url */
    sprintf(url, qweather_weather_forecast_url, qweather_city_id, CONFIG_APP_QWEATHER_KEY);
    const size_t resp_buf_size = 2048;
    char *response = malloc(resp_buf_size);
    size_t resp_size = network_https_request(response, resp_buf_size, qweather_weather_api_host, url, AAA_CA_CERT);

    if(resp_size) { //返回响应
        cJSON *root = cJSON_Parse(response);
        if(root == NULL) {
            code = 600;
        }
        /* 响应代码 */
        cJSON *resp_code = NULL;
        if(code == 200) {
            resp_code = cJSON_GetObjectItem(root, "code");
            if(resp_code == NULL) {
                code = 600;
            } else {
                code = atoi(cJSON_GetStringValue(resp_code));
            }
        }
        /* 天气预报信息 */
        cJSON *weather_days = NULL;
        if(code == 200) {
            weather_days = cJSON_GetObjectItem(root, "daily");
            if(weather_days == NULL) {
                code = 600;
            }
        }
        /* 第一天 */
        cJSON *weather_day1 = NULL;
        if(code == 200) {
            weather_day1 = cJSON_GetArrayItem(weather_days, 0);
            if(weather_day1 == NULL) {
                code = 600;
            }
        }
        /* 第一天最高温度 */
        cJSON *day1_high_temp = NULL;
        if(code == 200) {
            day1_high_temp = cJSON_GetObjectItem(weather_day1, "tempMax");
            if(day1_high_temp == NULL) {
                code = 600;
            } else {
                weather_forecast_info.day1_high_temp = atoi(cJSON_GetStringValue(day1_high_temp));
            }
        }
        /* 第一天最低温度 */
        cJSON *day1_low_temp = NULL;
        if(code == 200) {
            day1_low_temp = cJSON_GetObjectItem(weather_day1, "tempMin");
            if(day1_low_temp == NULL) {
                code = 600;
            } else {
                weather_forecast_info.day1_low_temp = atoi(cJSON_GetStringValue(day1_low_temp));
            }
        }
        /* 第一天天气 */
        cJSON *day1_weather = NULL;
        if(code == 200) {
            day1_weather = cJSON_GetObjectItem(weather_day1, "textDay");
            if(day1_weather == NULL) {
                code = 600;
            } else {
                strcpy(weather_forecast_info.day1_weather, cJSON_GetStringValue(day1_weather));
            }
        }
        /* 第二天 */
        cJSON *weather_day2 = NULL;
        if(code == 200) {
            weather_day2 = cJSON_GetArrayItem(weather_days, 1);
            if(weather_day2 == NULL) {
                code = 600;
            }
        }
        /* 第二天最高温度 */
        cJSON *day2_high_temp = NULL;
        if(code == 200) {
            day2_high_temp = cJSON_GetObjectItem(weather_day2, "tempMax");
            if(day2_high_temp == NULL) {
                code = 600;
            } else {
                weather_forecast_info.day2_high_temp = atoi(cJSON_GetStringValue(day2_high_temp));
            }
        }
        /* 第二天最低温度 */
        cJSON *day2_low_temp = NULL;
        if(code == 200) {
            day2_low_temp = cJSON_GetObjectItem(weather_day2, "tempMin");
            if(day2_low_temp == NULL) {
                code = 600;
            } else {
                weather_forecast_info.day2_low_temp = atoi(cJSON_GetStringValue(day2_low_temp));
            }
        }
        /* 第二天天气 */
        cJSON *day2_weather = NULL;
        if(code == 200) {
            day2_weather = cJSON_GetObjectItem(weather_day2, "textDay");
            if(day2_weather == NULL) {
                code = 600;
            } else {
                strcpy(weather_forecast_info.day2_weather, cJSON_GetStringValue(day2_weather));
            }
        }
        cJSON_Delete(root);
    } else { //未返回响应
        code = 500; //无响应或超时
    }

    if(code == 200) {
        ESP_LOGI(TAG, "forecast: %s, %d~%d℃; %s, %d~%d℃",
            weather_forecast_info.day1_weather, weather_forecast_info.day1_low_temp, weather_forecast_info.day1_high_temp,
            weather_forecast_info.day2_weather, weather_forecast_info.day2_low_temp, weather_forecast_info.day2_high_temp
        );
    } else { //响应代码无效
        ESP_LOGE(TAG, "error text: %s", network_qweather_get_error_text(code));
    }

    free(response);
    return code;
}

int network_qweather_update_now()
{
    int code;
    if(qweather_city_id == 0) { //还未获取城市信息
        ESP_LOGI(TAG, "getting city info...");
        code = network_qweather_get_city_id_name();
        
        if(code != 200) {
            return code;
        }
    }

    code = network_qweather_update_weather_now();
    if(code != 200) {
        return code;
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
