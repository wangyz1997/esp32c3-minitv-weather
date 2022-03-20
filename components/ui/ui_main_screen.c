#include "ui_main_screen.h"
#include "ui_qweather_icon.h"

#include <time.h>

#include "lv_fonts.h"

static lv_obj_t *main_screen;

static lv_obj_t *label_wifi_icon;

static lv_obj_t *label_time_hour;
static lv_obj_t *label_time_colon;
static lv_obj_t *label_time_minute;

static lv_obj_t *label_date;

static lv_obj_t *label_weather_icon;
static lv_obj_t *label_city;
static lv_obj_t *label_weather;
static lv_obj_t *image_gif;

static lv_obj_t *label_temperature;
static lv_obj_t *label_humidity;
static lv_obj_t *label_feels_like;

static lv_obj_t *label_forecast1_day;
static lv_obj_t *label_forecast1_temp;
static lv_obj_t *label_forecast1_weather;
static lv_obj_t *label_forecast2_day;
static lv_obj_t *label_forecast2_temp;
static lv_obj_t *label_forecast2_weather;

extern const uint8_t image_gif_start[] asm("_binary_miko_gif_start");

const lv_img_dsc_t image_gif_desc = {
    .header.always_zero = 0,
    .header.w = 0,
    .header.h = 0,
    .data_size = 0,
    .header.cf = LV_IMG_CF_RAW,
    .data = image_gif_start,
};

/**
 * @brief 创建主屏幕
 * 
 */
void ui_main_screen_create()
{
    /* 创建主屏幕 */
    main_screen = lv_obj_create(NULL);

    /* 右上图标 */
    label_wifi_icon = lv_label_create(main_screen); //WiFi图标
    lv_obj_set_width(label_wifi_icon, 28); //字符串宽度
    lv_obj_align(label_wifi_icon, LV_ALIGN_TOP_RIGHT, -2, 6); //字符串位置 左上角
    lv_obj_set_style_text_font(label_wifi_icon, &font_network_icons, 0);
    lv_obj_set_style_text_align(label_wifi_icon, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_text(label_wifi_icon, LV_SYMBOL_WIFI_FULL);

    uint16_t y_pos = 10; //纵向位置

    /* 时间 */
    label_time_colon = lv_label_create(main_screen); //时间的冒号
    lv_obj_set_width(label_time_colon, 24); //字符串宽度
    lv_obj_align(label_time_colon, LV_ALIGN_TOP_MID, 0, y_pos-4); //字符串位置 居中
    lv_obj_set_style_text_font(label_time_colon, &font_red_hat_mono_48, 0);
    lv_obj_set_style_text_align(label_time_colon, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(label_time_colon, ":");

    label_time_hour = lv_label_create(main_screen); //时间的小时
    lv_obj_set_width(label_time_hour, 60);
    lv_obj_align(label_time_hour, LV_ALIGN_TOP_MID, -40, y_pos); //向左偏移48像素
    lv_obj_set_style_text_font(label_time_hour, &font_red_hat_mono_48, 0);
    lv_obj_set_style_text_align(label_time_hour, LV_TEXT_ALIGN_RIGHT, 0); //字符串内右对齐
    lv_label_set_text(label_time_hour, "00");

    label_time_minute = lv_label_create(main_screen); //时间的分钟
    lv_obj_set_width(label_time_minute, 60);
    lv_obj_align(label_time_minute, LV_ALIGN_TOP_MID, 40, y_pos); //向右偏移48像素
    lv_obj_set_style_text_font(label_time_minute, &font_red_hat_mono_48, 0);
    lv_obj_set_style_text_align(label_time_minute, LV_TEXT_ALIGN_LEFT, 0); //字符串内左对齐
    lv_label_set_text(label_time_minute, "00");
    
    y_pos += 48;

    /* 日期与星期 */
    label_date = lv_label_create(main_screen);
    lv_obj_set_width(label_date, 240);
    lv_obj_align(label_date, LV_ALIGN_TOP_MID, 0, y_pos);
    lv_obj_set_style_text_font(label_date, &font_harmony_sans_20, 0);
    lv_obj_set_style_text_align(label_date, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(label_date, "");

    y_pos += 34;

    /* 天气图标 */
    label_weather_icon = lv_label_create(main_screen); //天气图标
    lv_obj_set_width(label_weather_icon, 50); //字符串宽度
    lv_obj_align(label_weather_icon, LV_ALIGN_TOP_MID, -80, y_pos);
    lv_obj_set_style_text_font(label_weather_icon, &font_qweather_icons, 0);
    lv_obj_set_style_text_align(label_weather_icon, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_text(label_weather_icon, "\uF146");

    /* 城市与天气文本 */
    label_city = lv_label_create(main_screen); //城市名称
    lv_obj_set_width(label_city, 80); //字符串宽度
    lv_obj_align(label_city, LV_ALIGN_TOP_MID, 0, y_pos);
    lv_obj_set_style_text_font(label_city, &font_harmony_sans_20, 0);
    lv_obj_set_style_text_align(label_city, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(label_city, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(label_city, "--");

    label_weather = lv_label_create(main_screen); //天气
    lv_obj_set_width(label_weather, 80); //字符串宽度
    lv_obj_align(label_weather, LV_ALIGN_TOP_MID, 0, y_pos+28);
    lv_obj_set_style_text_font(label_weather, &font_harmony_sans_20, 0);
    lv_obj_set_style_text_align(label_weather, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(label_weather, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(label_weather, "--");

    /* gif动图 */
    image_gif = lv_gif_create(main_screen);
    lv_gif_set_src(image_gif, &image_gif_desc);
    lv_obj_align(image_gif, LV_ALIGN_TOP_MID, 80, y_pos);

    y_pos += 56;

    /* 温湿度与体感温度 */
    label_temperature = lv_label_create(main_screen); //温度
    lv_obj_set_width(label_temperature, 80); //字符串宽度
    lv_obj_align(label_temperature, LV_ALIGN_TOP_MID, -80, y_pos);
    lv_obj_set_style_text_font(label_temperature, &font_harmony_sans_20, 0);
    lv_obj_set_style_text_align(label_temperature, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_recolor(label_temperature, true);
    lv_label_set_text(label_temperature, "#993300 \ue900# --℃");
    
    label_humidity = lv_label_create(main_screen); //湿度
    lv_obj_set_width(label_humidity, 80); //字符串宽度
    lv_obj_align(label_humidity, LV_ALIGN_TOP_MID, 0, y_pos);
    lv_obj_set_style_text_font(label_humidity, &font_harmony_sans_20, 0);
    lv_obj_set_style_text_align(label_humidity, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_recolor(label_humidity, true);
    lv_label_set_text(label_humidity, "#0099CC \ue901# --%");
    
    label_feels_like = lv_label_create(main_screen); //体感温度
    lv_obj_set_width(label_feels_like, 80); //字符串宽度
    lv_obj_align(label_feels_like, LV_ALIGN_TOP_MID, 80, y_pos);
    lv_obj_set_style_text_font(label_feels_like, &font_harmony_sans_20, 0);
    lv_obj_set_style_text_align(label_feels_like, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_recolor(label_feels_like, true);
    lv_label_set_text(label_feels_like, "#009900 \ue902# --℃");
    
    y_pos += 34;

    /* 天气预报-明天 */
    label_forecast1_day = lv_label_create(main_screen); //明天文本
    lv_obj_set_width(label_forecast1_day, 40); //字符串宽度
    lv_obj_align(label_forecast1_day, LV_ALIGN_TOP_MID, -90, y_pos);
    lv_obj_set_style_text_font(label_forecast1_day, &font_harmony_sans_20, 0);
    lv_obj_set_style_text_align(label_forecast1_day, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(label_forecast1_day, "明天");
    
    label_forecast1_weather = lv_label_create(main_screen); //天气
    lv_obj_set_width(label_forecast1_weather, 80); //字符串宽度
    lv_obj_align(label_forecast1_weather, LV_ALIGN_TOP_MID, -20, y_pos);
    lv_obj_set_style_text_font(label_forecast1_weather, &font_harmony_sans_20, 0);
    lv_obj_set_style_text_align(label_forecast1_weather, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(label_forecast1_weather, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(label_forecast1_weather, "--");
    
    label_forecast1_temp = lv_label_create(main_screen); //温度范围
    lv_obj_set_width(label_forecast1_temp, 90); //字符串宽度
    lv_obj_align(label_forecast1_temp, LV_ALIGN_TOP_MID, 70, y_pos);
    lv_obj_set_style_text_font(label_forecast1_temp, &font_harmony_sans_20, 0);
    lv_obj_set_style_text_align(label_forecast1_temp, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(label_forecast1_temp, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(label_forecast1_temp, "--");
    
    y_pos += 28;

    /* 天气预报-后天 */
    label_forecast2_day = lv_label_create(main_screen); //后天文本
    lv_obj_set_width(label_forecast2_day, 40); //字符串宽度
    lv_obj_align(label_forecast2_day, LV_ALIGN_TOP_MID, -90, y_pos);
    lv_obj_set_style_text_font(label_forecast2_day, &font_harmony_sans_20, 0);
    lv_obj_set_style_text_align(label_forecast2_day, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(label_forecast2_day, "后天");
    
    label_forecast2_weather = lv_label_create(main_screen); //天气
    lv_obj_set_width(label_forecast2_weather, 80); //字符串宽度
    lv_obj_align(label_forecast2_weather, LV_ALIGN_TOP_MID, -20, y_pos);
    lv_obj_set_style_text_font(label_forecast2_weather, &font_harmony_sans_20, 0);
    lv_obj_set_style_text_align(label_forecast2_weather, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(label_forecast2_weather, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(label_forecast2_weather, "--");

    label_forecast2_temp = lv_label_create(main_screen); //温度范围
    lv_obj_set_width(label_forecast2_temp, 90); //字符串宽度
    lv_obj_align(label_forecast2_temp, LV_ALIGN_TOP_MID, 70, y_pos);
    lv_obj_set_style_text_font(label_forecast2_temp, &font_harmony_sans_20, 0);
    lv_obj_set_style_text_align(label_forecast2_temp, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(label_forecast2_temp, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(label_forecast2_temp, "--");
}

void ui_main_screen_load()
{
    lv_scr_load(main_screen);
}

/**
 * @brief 更新主界面的时间与日期
 * 
 */
void ui_main_screen_timedate_update()
{
    const char *weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
    static struct tm tm_last = {
        .tm_min = -1,
        .tm_mday = -1,
    };

    time_t now;
    time(&now);
    struct tm *tm_now = localtime(&now);

    if(tm_last.tm_min != tm_now->tm_min) { //分变化，更新时间
        lv_label_set_text_fmt(label_time_hour, "%02d", tm_now->tm_hour);
        lv_label_set_text_fmt(label_time_minute, "%02d", tm_now->tm_min);
    }
    
    if(tm_last.tm_mday != tm_now->tm_mday) { //日变化，更新日期
        lv_label_set_text_fmt(label_date, "%d月%d日  周%s", tm_now->tm_mon+1, tm_now->tm_mday, weekdays[tm_now->tm_wday]);
    }

    mempcpy(&tm_last, tm_now, sizeof(tm_last)); //保存上一次执行时的时间
}

void ui_main_screen_weather_now_update(const weather_now_info_t *weather_now_info)
{
    lv_label_set_text(label_weather, weather_now_info->weather);
    lv_label_set_text_fmt(label_temperature, "#993300 \ue900# %d℃", weather_now_info->temperature);
    lv_label_set_text_fmt(label_humidity, "#0099CC \ue901# %d%%", weather_now_info->humidity);
    lv_label_set_text_fmt(label_feels_like, "#009900 \ue902# %d℃", weather_now_info->feels_like);
    lv_label_set_text(label_weather_icon, ui_qweather_icon_get_from_code(weather_now_info->weather_icon));
}

void ui_main_screen_weather_forecast_update(const weather_forecast_info_t *weather_forecast_info)
{
    lv_label_set_text(label_forecast1_weather, weather_forecast_info->day1_weather);
    lv_label_set_text_fmt(label_forecast1_temp, "%d~%d℃", weather_forecast_info->day1_low_temp, weather_forecast_info->day1_high_temp);
    lv_label_set_text(label_forecast2_weather, weather_forecast_info->day2_weather);
    lv_label_set_text_fmt(label_forecast2_temp, "%d~%d℃", weather_forecast_info->day2_low_temp, weather_forecast_info->day2_high_temp);
}

void ui_main_screen_city_update(const char *city)
{
    lv_label_set_text(label_city, city);
}

