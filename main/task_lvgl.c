#include "task_lvgl.h"
#include "task_app.h"

#include "lv_port_disp.h"
#include "lv_port_tick.h"
#include "lv_fonts.h"

static SemaphoreHandle_t lvgl_semphr;

static void task_handler(void *_args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(10);

	lv_init();
    lv_port_disp_init();
    lv_create_tick();
    
    lv_port_font_harmony_sans_20_load("font_hs20");

    task_lvgl_semphr_init();
    task_app_create();

    while(1) {
        task_lvgl_semphr_take();
        lv_task_handler();
        task_lvgl_semphr_give();

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

void task_lvgl_semphr_init()
{
    vSemaphoreCreateBinary(lvgl_semphr);
}

void task_lvgl_semphr_take()
{
    xSemaphoreTake(lvgl_semphr, portMAX_DELAY);
}

void task_lvgl_semphr_give()
{
    xSemaphoreGive(lvgl_semphr);
}

void task_lvgl_create()
{
    xTaskCreate(task_handler, "LVGL Task", 4096, NULL, 4, NULL);
}
