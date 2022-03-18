#include "bsp_lcd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_system.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/spi_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "sdkconfig.h"

const static char *TAG = "bsp_lcd";

/**
 * @brief SPI传输指令结构体，由于传输函数返回时SPI控制器仍在使用DMA传输，因此使用静态变量在全局区申请
 * 以保证在函数返回时此变量依然可用。
 */
static spi_transaction_t lcd_trans[6]; //5个指令传输+1个数据传输

static void lcd_spi_pre_transfer_callback(spi_transaction_t *spi);

/**
 * @brief SPI总线配置
 */
static const spi_bus_config_t lcd_spi_bus_config = {
    .miso_io_num = -1, //未使用
    .mosi_io_num = CONFIG_BSP_LCD_PIN_MOSI,
    .sclk_io_num = CONFIG_BSP_LCD_PIN_CLK,
    .quadwp_io_num = -1, //未使用
    .quadhd_io_num = -1, //未使用
    .max_transfer_sz = BSP_LCD_X_PIXELS*BSP_LCD_Y_PIXELS*2, //最大传输大小为整个屏幕的像素数*2个字节
};

/**
 * @brief LCD外设配置
 */
static const spi_device_interface_config_t lcd_spi_driver_config = {
    .clock_speed_hz = CONFIG_BSP_LCD_SPI_CLOCK_MHZ*1000000, //SPI总线时钟频率，Hz
    .mode = 0, //SPI模式0，CPOL=0，CPHA=0
    .spics_io_num = CONFIG_BSP_LCD_PIN_CS, //CS引脚
    .queue_size = sizeof(lcd_trans)/sizeof(spi_transaction_t), //传输队列深度
    .pre_cb = lcd_spi_pre_transfer_callback, //SPI每次传输前的回调函数，用于控制D/C脚
};

/**
 * @brief 初始化LCD所用的总线，并返回SPI句柄
 * 
 * @return SPI句柄
 */
static spi_device_handle_t lcd_bus_init()
{
    /* 创建SPI句柄 */
    spi_device_handle_t spi;

    /* 根据buscfg的内容初始化SPI总线 */
    ESP_ERROR_CHECK(
        spi_bus_initialize(BSP_LCD_SPI_HOST, &lcd_spi_bus_config, SPI_DMA_CH_AUTO)
    );

    /* 根据lcdcfg的内容将LCD挂载到SPI总线上 */
    ESP_ERROR_CHECK(
        spi_bus_add_device(BSP_LCD_SPI_HOST, &lcd_spi_driver_config, &spi)
    );

    return spi;
}

/**
 * @brief SPI开始一次传输前的回调函数，用于控制D/C引脚
 * 
 * @param spi SPI传输指令结构体
 */
static void lcd_spi_pre_transfer_callback(spi_transaction_t *trans)
{
    uint32_t dc = (uint32_t)(trans->user);
    /* 设置D/C引脚的电平 */
    gpio_set_level(CONFIG_BSP_LCD_PIN_DC, dc);
}

/**
 * @brief 使用查询方式向LCD发送一个指令，用于LCD初始化阶段
 * 
 * @param spi SPI句柄
 * @param cmd 要发送的指令字节
 */
static void lcd_send_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    /* 创建并初始化SPI传输指令结构体 */
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));

    /* 配置传输 */
    trans.length = 8; //指令为1字节8位
    trans.tx_data[0] = cmd; //指令
    trans.user = (void*)0; //D/C=0
    trans.flags = SPI_TRANS_USE_TXDATA; //发送tx_data内的数据

    /* 启动传输 */
    ESP_ERROR_CHECK(
        spi_device_polling_transmit(spi, &trans)
    );
}

/**
 * @brief 使用查询方式向LCD发送一组数据，用于LCD初始化阶段
 * 
 * @param spi SPI句柄
 * @param data 要发送的数据指针
 * @param len 数据长度
 */
static void lcd_send_data(spi_device_handle_t spi, const uint8_t *data, uint8_t len)
{
    /* 创建并初始化SPI传输指令结构体 */
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));

    /* 配置传输 */
    trans.length = len*8; //数据长度，单位为位
    trans.tx_buffer = data;
    trans.user = (void*)1; //D/C=1
    
    /* 启动传输 */
    ESP_ERROR_CHECK(
        spi_device_polling_transmit(spi, &trans)
    );
}

/**
 * @brief 初始化ST7789v控制器的寄存器
 * 
 * @param spi SPI句柄
 */
static void lcd_reg_init_st7789v(spi_device_handle_t spi)
{
    /* 退出睡眠模式 */
    lcd_send_cmd(spi, 0x11);
    /* IPS屏幕需要设置屏幕反显才能显示正确的颜色 */
    lcd_send_cmd(spi, 0x21); //开启反显
    /* 显存访问控制 */
    lcd_send_cmd(spi, 0x36);
    lcd_send_data(spi, (uint8_t[]){0x00}, 1); //MX=MV=MY=ML=MH=0, RGB=0 竖屏，芯片在下
    /* 像素格式 */
    lcd_send_cmd(spi, 0x3A);
    lcd_send_data(spi, (uint8_t[]){0x05}, 1); //MCU mode, 16bit/pixel
    /* Porch设置 */
    lcd_send_cmd(spi, 0xB2);
    lcd_send_data(spi, (uint8_t[]){0x0C, 0x0C, 0x00, 0x33, 0x33}, 5);
    /* Gate设置 */
    lcd_send_cmd(spi, 0xB7);
    lcd_send_data(spi, (uint8_t[]){0x35}, 1); //Vgh=13.26V, Vgl=-10.43V
    /* VCOM设置 */
    lcd_send_cmd(spi, 0xBB);
    lcd_send_data(spi, (uint8_t[]){0x19}, 1); //VCOM=0.725V 
    /* LCN设置 */
    lcd_send_cmd(spi, 0xC0);
    lcd_send_data(spi, (uint8_t[]){0x2C}, 1); //XOR: BGR, MX, MH 
    /* VDV与VRH写使能 */
    lcd_send_cmd(spi, 0xC2);
    lcd_send_data(spi, (uint8_t[]){0x01, 0xFF}, 2); //CMDEN=1
    /* VRH设置 */
    lcd_send_cmd(spi, 0xC3);
    lcd_send_data(spi, (uint8_t[]){0x12}, 1); //Vrh=4.45+
    /* VDV设置 */
    lcd_send_cmd(spi, 0xC4);
    lcd_send_data(spi, (uint8_t[]){0x20}, 1); //Vdv=0
    /* 刷新率设置 */
    lcd_send_cmd(spi, 0xC6);
    lcd_send_data(spi, (uint8_t[]){0x0F}, 1); //60Hz, no column inversion
    /* 电源控制1 */
    lcd_send_cmd(spi, 0xD0);
    lcd_send_data(spi, (uint8_t[]){0xA4, 0xA1}, 2); //AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V
    /* 正电压伽马控制 */
    lcd_send_cmd(spi, 0xE0);
    lcd_send_data(spi, (uint8_t[]){0xD0, 0x0D, 0x14, 0x0B, 0x0B, 0x07, 0x3A, 0x44, 0x50, 0x08, 0x13, 0x13, 0x2D, 0x32}, 14);
    /* 负电压伽马控制 */
    lcd_send_cmd(spi, 0xE1);
    lcd_send_data(spi, (uint8_t[]){0xD0, 0x0D, 0x14, 0x0B, 0x0B, 0x07, 0x3A, 0x44, 0x50, 0x08, 0x13, 0x13, 0x2D, 0x32}, 14);
}

/**
 * @brief 在LCD上绘制矩形区域，使用DMA传输
 * 
 * @param spi SPI句柄
 * @param x0 矩形的左上角横坐标
 * @param y0 矩阵的左上角纵坐标
 * @param x1 矩形的右下角横坐标
 * @param y1 矩形的右下角纵坐标
 * @param dat 要显示的数据
 */
void bsp_lcd_draw_rect(spi_device_handle_t spi, uint16_t x0, uint16_t y0, uint16_t x1,
                  uint16_t y1, const uint8_t *dat)
{
    /* 由于存储在静态全局区，因此每次传输都需要初始化SPI传输指令结构体 */
    memset(&lcd_trans, 0, sizeof(lcd_trans)); //前5个传输用于设置区域命令，第6个传输用于传输矩形的显示数据

    /**
    * SPI使用DMA传输，而DMA只支持内部RAM作为buffer。若传入的指针为非内部RAM区（PSRAM/ROM），
    * 则idf会自动重新申请一段位于DMA可用区域的内存，并将传入的数据复制到新申请的内存中。
    * 该内存在spi_device_get_trans_result函数中被释放，因此传输完成后必须调用传输结束函数。
    */

    /* 第1次传输：指令，设置Column地址 */
    lcd_trans[0].tx_data[0] = 0x2A;
    lcd_trans[0].length = 8; //传输长度：1字节8位
    lcd_trans[0].user = (void*)0; //D/C=0，指令
    lcd_trans[0].flags = SPI_TRANS_USE_TXDATA; //发送tx_data内的数据
    ESP_ERROR_CHECK(
        spi_device_queue_trans(spi, &lcd_trans[0], 0) //队列长度足够，不阻塞直接发送，提高速度
    );

    /* 第2次传输：数据，Column范围 */
    lcd_trans[1].tx_data[0] = x0>>8; //起始地址高8位
    lcd_trans[1].tx_data[1] = x0&0xFF; //起始地址高8位
    lcd_trans[1].tx_data[2] = x1>>8; //结束地址高8位
    lcd_trans[1].tx_data[3] = x1&0xFF; //结束地址高8位
    lcd_trans[1].length = 4*8;
    lcd_trans[1].user = (void*)1; //D/C=1，数据
    lcd_trans[1].flags = SPI_TRANS_USE_TXDATA;
    ESP_ERROR_CHECK(
        spi_device_queue_trans(spi, &lcd_trans[1], 0)
    );

    /* 第3次传输：指令，设置Page地址 */
    lcd_trans[2].tx_data[0] = 0x2B;
    lcd_trans[2].length = 8;
    lcd_trans[2].user = (void*)0;
    lcd_trans[2].flags = SPI_TRANS_USE_TXDATA;
    ESP_ERROR_CHECK(
        spi_device_queue_trans(spi, &lcd_trans[2], 0)
    );

    /* 第4次传输：数据，Page范围 */
    lcd_trans[3].tx_data[0] = y0>>8;
    lcd_trans[3].tx_data[1] = y0&0xFF;
    lcd_trans[3].tx_data[2] = y1>>8;
    lcd_trans[3].tx_data[3] = y1&0xFF;
    lcd_trans[3].length = 4*8;
    lcd_trans[3].user = (void*)1;
    lcd_trans[3].flags = SPI_TRANS_USE_TXDATA;
    ESP_ERROR_CHECK(
        spi_device_queue_trans(spi, &lcd_trans[3], 0)
    );

    /* 第5次传输：指令，开始写入图像数据 */
    lcd_trans[4].tx_data[0] = 0x2C;
    lcd_trans[4].length = 8;
    lcd_trans[4].user = (void*)0;
    lcd_trans[4].flags = SPI_TRANS_USE_TXDATA;
    ESP_ERROR_CHECK(
        spi_device_queue_trans(spi, &lcd_trans[4], 0)
    );

    /* 第6次传输：数据，图像数据 */
    lcd_trans[5].tx_buffer = dat;
    lcd_trans[5].length = ((x1-x0+1)*(y1-y0+1)*2)*8;
    lcd_trans[5].user = (void*)1;
    lcd_trans[5].flags = 0; //发送tx_buffer指向的地址
    ESP_ERROR_CHECK(
        spi_device_queue_trans(spi, &lcd_trans[5], 0)
    );

    /**
     * 此处所有指令已经写入队列，SPI控制器正在后台使用DMA进行发送。在下一次传输之前，需要调用
     * lcd_draw_rect_wait_busy()函数来等待上次传输完成并释放内存。
     */
}

/**
 * @brief 等待LCD矩形区域绘制完成，并释放内存
 * 
 * @param spi SPI句柄
 */
void bsp_lcd_draw_rect_wait_busy(spi_device_handle_t spi)
{
    /**
     * @brief 保存传输结果的指令结构体，会返回指向存放接收到数据的指针，由于驱动LCD的SPI总线是只写的，
     * 因此此结构体只做占位之用。
     */
    spi_transaction_t *rtrans;

    /* 查询并阻塞等待所有传输结束 */
    for(int i = 0; i < sizeof(lcd_trans)/sizeof(spi_transaction_t); i ++) {
        ESP_ERROR_CHECK(
            spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY)
        );
    }
}

/**
 * @brief 开关LCD的显示
 * 
 * @param spi SPI句柄
 * @param status 真或假控制LCD的开或关
 */
void bsp_lcd_display_switch(spi_device_handle_t spi, bool status)
{
    if(status) {
        lcd_send_cmd(spi, 0x29); //开显示
        bsp_lcd_set_backlight(80); //打开背光
        ESP_LOGI(TAG, "lcd display on");
    } else {
        lcd_send_cmd(spi, 0x28); //关显示
        bsp_lcd_set_backlight(0); //关闭背光
        ESP_LOGW(TAG, "lcd display off");
    }
}

/* 定时器配置 */
static const ledc_timer_config_t lcd_bl_ledc_timer = {
   .duty_resolution = LEDC_TIMER_10_BIT, //LEDC驱动器占空比精度
   .freq_hz = 2000, //PWM频率
   .speed_mode = LEDC_LOW_SPEED_MODE,
   .timer_num = LEDC_TIMER_0, //ledc使用的定时器编号。若需要生成多个频率不同的PWM信号，则需要指定不同的定时器
   .clk_cfg = LEDC_AUTO_CLK, //自动选择定时器的时钟源
};

/* ledc通道配置 */
static const ledc_channel_config_t lcd_bl_ledc_channel = {
   .channel    = LEDC_CHANNEL_0, //LCD背光使用通道0
   .duty       = 0, //占空比0
   .gpio_num   = CONFIG_BSP_LCD_PIN_BL, //连接背光的IO
   .speed_mode = LEDC_LOW_SPEED_MODE,
   .hpoint     = 0,
   .timer_sel  = LEDC_TIMER_0, //使用上面初始化过的定时器
};

static const gpio_config_t lcd_io_conf = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 1ull<<CONFIG_BSP_LCD_PIN_DC | 1ull<<CONFIG_BSP_LCD_PIN_RST,
    .pull_up_en = 0,
    .pull_down_en = 0,
};

/**
 * @brief 初始化LCD
 * 
 * @return spi_device_handle_t 用于控制LCD的SPI句柄
 */
spi_device_handle_t bsp_lcd_init()
{
    /* 初始化非SPI的GPIO */
    gpio_config(&lcd_io_conf);
    /* 复位LCD */
    gpio_set_level(CONFIG_BSP_LCD_PIN_RST, 0); //复位
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(CONFIG_BSP_LCD_PIN_RST, 1); //复位
    vTaskDelay(pdMS_TO_TICKS(50));

    /* 初始化总线 */
    spi_device_handle_t spi = lcd_bus_init();
    /* 初始化LCD控制器寄存器 */
    lcd_reg_init_st7789v(spi);

    /* 初始化LCD背光 */
    ledc_timer_config(&lcd_bl_ledc_timer); //初始化定时器1
    ledc_channel_config(&lcd_bl_ledc_channel); //将初始化好的定时器编号传给ledc通道初始化函数

    ESP_LOGI(TAG, "lcd bus & regs initialized");
    return spi;
}

/**
 * @brief 设置背光亮度
 * 
 * @param brightness 背光百分比，0-100
 * @return esp_err_t 错误代码
 */
void bsp_lcd_set_backlight(uint8_t brightness)
{
    /* 设定PWM占空比 */
    ledc_set_duty(lcd_bl_ledc_channel.speed_mode, lcd_bl_ledc_channel.channel, brightness*10);
    /* 更新PWM占空比输出 */
    ledc_update_duty(lcd_bl_ledc_channel.speed_mode, lcd_bl_ledc_channel.channel);
}
