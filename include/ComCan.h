#ifndef CAN_COMMS_H
#define CAN_COMMS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ComCfg.h"
#include <ESP32CAN.h>
#include "RTE.h"

#define COM_CAN_TASK_DELAY_TIME_MS 2

void ComCan_init(void);

void ComCan_receive(void* param);
void ComCan_transmit(void* param);

#endif