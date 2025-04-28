
/*
*
* Dimmer handler
* Controls dimmer based on GPIO and CAN inputs
*
*/

#pragma once

#include "vars.h"

#define DIMH_API_INIT_U32                  ((uint32_t) 1)
#define DIMH_API_NVSM_RUN_U32              ((uint32_t) 2)

typedef struct
{
} tDIMH_INITDATA_STR;

void dimh_init(tDIMH_INITDATA_STR* dimhCfg);
void dimh_run_5ms(void);