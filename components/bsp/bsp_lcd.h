#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include "driver/spi_master.h"

/* 屏幕总线参数 */
#define BSP_LCD_SPI_HOST       SPI2_HOST //即VSPI

/* 屏幕分辨率 */
#define BSP_LCD_X_PIXELS 240
#define BSP_LCD_Y_PIXELS 240

spi_device_handle_t bsp_lcd_init();
void bsp_lcd_display_switch(spi_device_handle_t spi, bool status);
void bsp_lcd_set_backlight(uint8_t brightness);
void bsp_lcd_draw_rect(spi_device_handle_t spi, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const uint8_t *dat);
void bsp_lcd_draw_rect_wait_busy(spi_device_handle_t spi);

#endif
