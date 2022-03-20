#include "ui_connecting_screen.h"

#include "lv_fonts.h"

static lv_obj_t *connecting_screen;

static lv_obj_t *label_connecting;
static lv_obj_t *spinner_connecting;

/**
 * @brief 显示WiFi连接屏幕
 * 
 */
void ui_connecting_screen_create()
{
    connecting_screen = lv_obj_create(NULL);

    label_connecting = lv_label_create(connecting_screen);
    lv_obj_set_width(label_connecting, 200);
    lv_obj_set_style_text_font(label_connecting, &font_harmony_sans_20, 0);
    lv_obj_align(label_connecting, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_style_text_align(label_connecting, LV_TEXT_ALIGN_CENTER, 0);
    
    spinner_connecting = lv_spinner_create(connecting_screen, 1000, 60);
    lv_obj_set_size(spinner_connecting, 50, 50);
    lv_obj_align(spinner_connecting, LV_ALIGN_CENTER, 0, 40);
}

void ui_connecting_screen_load()
{
    lv_scr_load(connecting_screen);
}

void ui_connecting_screen_update(ui_connecting_screen_status_t status)
{
    switch (status)
    {
    case status_waiting_wifi:
        lv_label_set_text(label_connecting, "正在连接WiFi...");
        lv_obj_clear_flag(spinner_connecting, LV_OBJ_FLAG_HIDDEN);
        break;
    case status_fetching_data:
        lv_label_set_text(label_connecting, "正在获取数据...");
        lv_obj_clear_flag(spinner_connecting, LV_OBJ_FLAG_HIDDEN);
        break;

    case status_data_error:
        lv_label_set_text(label_connecting, "数据获取失败");
        lv_obj_add_flag(spinner_connecting, LV_OBJ_FLAG_HIDDEN);
        break;
    
    default:
        break;
    }
}
