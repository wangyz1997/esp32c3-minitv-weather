#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include <driver/gpio.h>

void bsp_key_init();
bool bsp_key_read();

#endif
