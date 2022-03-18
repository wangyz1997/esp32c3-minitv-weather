#include "bsp_key.h"

#include "sdkconfig.h"

static const gpio_config_t key_io_conf = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = 1ull<<CONFIG_BSP_KEY_PIN,
    .pull_up_en = 1,
    .pull_down_en = 0,
};

void bsp_key_init()
{
    gpio_config(&key_io_conf);
}

bool bsp_key_read()
{
    return gpio_get_level(CONFIG_BSP_KEY_PIN) == 0;
}
