/*
*   Modbus control module
*   used to communicate with Modbus devices
*
*/

#pragma once

#include "rtdb.h"
#include "errh.h"
#include "driver/uart.h"

#define MBCM_API_INIT_U32                  ((uint32_t) 1)

#define MBCM_MB1_BAUDRATE_U32   ((uint32_t) 256000)   ///< baudrate for Modbus 1
#define MBCM_MB2_BAUDRATE_U32   ((uint32_t)   9600)   ///< baudrate for Modbus 2

#define MBCM_MB1_UART_NUM_STR       (UART_NUM_1)   ///< UART number for Modbus 1
#define MBCM_MB2_UART_NUM_STR       (UART_NUM_2)   ///< UART number for Modbus 1

typedef struct
{
    uint32_t nr_moduleId_U32;       ///< ID of the module
} tMBCM_INITDATA_STR;

void mbcm_init(tMBCM_INITDATA_STR* DiomCfg);