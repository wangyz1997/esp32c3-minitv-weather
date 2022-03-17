#ifndef __LV_PORT_DISP_H
#define __LV_PORT_DISP_H

#include "lvgl.h"

#define LV_PORT_DISP_BUFFER_LINES 40 //缓冲区大小为LCD横向分辨率*40行

void lv_port_disp_init(void);

#endif
