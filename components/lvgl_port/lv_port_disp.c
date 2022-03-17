#include "lv_port_disp.h"
#include <esp_system.h>
#include <esp_log.h>
#include "bsp_lcd.h"

static DMA_ATTR lv_color_t lvgl_draw_buff1[BSP_LCD_X_PIXELS*LV_PORT_DISP_BUFFER_LINES];
static DMA_ATTR lv_color_t lvgl_draw_buff2[BSP_LCD_X_PIXELS*LV_PORT_DISP_BUFFER_LINES];

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    /* 等待传输完成 */
    bsp_lcd_draw_rect_wait_busy(disp_drv->user_data);
    /* 通知lvgl传输已完成 */
    lv_disp_flush_ready(disp_drv);

    /* 启动新的传输 */
    bsp_lcd_draw_rect(disp_drv->user_data, area->x1, area->y1, area->x2, area->y2, (const uint8_t*)color_p);
}

void lv_port_disp_init()
{
    /* 初始化LCD总线与寄存器 */
    spi_device_handle_t lcd_spi = bsp_lcd_init();

    /* 向lvgl注册缓冲区 */
    static lv_disp_draw_buf_t draw_buf_dsc; //需要全程生命周期，设置为静态变量
    lv_disp_draw_buf_init(&draw_buf_dsc, lvgl_draw_buff1, lvgl_draw_buff2, BSP_LCD_X_PIXELS*LV_PORT_DISP_BUFFER_LINES);
    /* 释放信号量 */
    bsp_lcd_draw_rect(lcd_spi, 0, 0, 0, 0, (const uint8_t*)lvgl_draw_buff1);
    
    /* 创建并初始化用于在LVGL中注册显示设备的结构 */
    static lv_disp_drv_t disp_drv; //显示设备描述结构
    lv_disp_drv_init(&disp_drv); //使用默认值初始化该结构
    /* 设置显示分辨率 */
    disp_drv.hor_res = BSP_LCD_X_PIXELS;
    disp_drv.ver_res = BSP_LCD_Y_PIXELS;
    /* 将总线句柄放入lv_disp_drv_t中用户自定义段 */
    disp_drv.user_data = lcd_spi;

    /* 设置显示函数 用于在将矩形缓冲区刷新到屏幕上 */
    disp_drv.flush_cb = disp_flush;
    /* 设置缓冲区 */
    disp_drv.draw_buf = &draw_buf_dsc;
    /* 注册显示设备 */
    lv_disp_drv_register(&disp_drv);

    /* 开显示 */
    bsp_lcd_display_switch(lcd_spi, true);
}
