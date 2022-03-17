#ifndef __TASK_NETWORK_H
#define __TASK_NETWORK_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#define NETWORK_CONNECTED_EVT   BIT0
#define NETWORK_DATA_READY_EVT  BIT1
#define NETWORK_DATA_ERROR_EVT  BIT2

extern EventGroupHandle_t network_event_group;

void task_network_create();

#endif
