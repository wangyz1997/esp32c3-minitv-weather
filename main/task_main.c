#include <esp_err.h>
#include <esp_log.h>

#include "task_lvgl.h"
#include "task_network.h"

void app_main(void)
{
    task_network_create();
    task_lvgl_create();

    // while(1) {
    //     ESP_LOGI("free heap", "%.1fkb", (float)esp_get_free_heap_size()/1024);

    //     vTaskDelay(pdMS_TO_TICKS(5000));
    // }

    vTaskDelete(NULL);
}
