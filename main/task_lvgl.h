#ifndef __TASK_LVGL_H
#define __TASK_LVGL_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

void task_lvgl_create();
void task_lvgl_semphr_init();
void task_lvgl_semphr_take();
void task_lvgl_semphr_give();

#endif
