#ifndef __LVGL_FONT_H
#define __LVGL_FONT_H

#include "lvgl.h"

#define LV_SYMBOL_TEMPERATURE     "\ue900" /* 温度 */
#define LV_SYMBOL_HUMIDITY        "\ue901" /* 湿度 */
#define LV_SYMBOL_FEELS_LIKE      "\ue902" /* 体感温度 */
#define LV_SYMBOL_WIFI_FULL       "\ue903" /* WiFi优 */
#define LV_SYMBOL_WIFI_HALF       "\ue904" /* WiFi良 */
#define LV_SYMBOL_WIFI_LOW        "\ue905" /* WiFi差 */

extern const lv_font_t font_harmony_sans_20;
extern const lv_font_t font_red_hat_mono_48;
extern const lv_font_t font_ui_icons;
extern const lv_font_t font_qweather_icons;
extern const lv_font_t font_network_icons;

void lv_port_font_harmony_sans_20_load(const char *partition_label);

#endif
