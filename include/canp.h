/*
*
*   Can port
*
*/
#pragma once
#include "vars.h"
#include "canm.h"

#if defined(PLATFORM_ESP32)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/twai.h>    // CAN protocol library, a.k.a. twin wire automotive interface
#include "errh.h"
#elif defined(PLATFORM_LOCALHOST)
    // mock headers or stubs
#endif

#define CANP_API_INIT_U32                  ((uint32_t) 1)

typedef struct
{
    uint32_t ti_ms_taskDelay_U32;   ///< time in ms between task executions
} tCANP_INITDATA_STR;

tB canp_init(tCANP_INITDATA_STR* CanpCfg);
tB canp_send(const tCANM_MSG *msg);
tB canp_receive(tCANM_MSG *msg);