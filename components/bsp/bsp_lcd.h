#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include "driver/spi_master.h"

/* 屏幕总线参数 */
#define BSP_LCD_SPI_HOST       SPI2_HOST //即VSPI
#define BSP_LCD_SPI_CLOCK_MHZ  40        //LCD所用SPI总线的时钟频率

/* 屏幕硬件连接 */
#define BSP_LCD_PIN_CS   6
#define BSP_LCD_PIN_CLK  5
#define BSP_LCD_PIN_MOSI 2
#define BSP_LCD_PIN_DC   7
#define BSP_LCD_PIN_RST  1
#define BSP_LCD_PIN_BL   10

/* 屏幕分辨率 */
#define BSP_LCD_X_PIXELS 240
#define BSP_LCD_Y_PIXELS 240

spi_device_handle_t bsp_lcd_init();
void bsp_lcd_display_switch(spi_device_handle_t spi, bool status);
void bsp_lcd_set_backlight(uint8_t brightness);
void bsp_lcd_draw_rect(spi_device_handle_t spi, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const uint8_t *dat);
void bsp_lcd_draw_rect_wait_busy(spi_device_handle_t spi);

#endif
