#include <esp_err.h>
#include <esp_log.h>

#include "task_ui.h"

#include "bsp_key.h"

void app_main(void)
{
    bsp_key_init();

    task_ui_create();

    // while(1) {
    //     ESP_LOGI("free heap", "%.1fkb", (float)esp_get_free_heap_size()/1024);

    //     vTaskDelay(pdMS_TO_TICKS(5000));
    // }

    vTaskDelete(NULL);
}
