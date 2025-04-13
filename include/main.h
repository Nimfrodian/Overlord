#pragma once

#include "espa.h"
#include "stdint.h"
#include "string.h"

#include "canm.h" ///< CAN module
#include "rtdb.h" ///< Real Time Database module
#include "dmas.h" ///< Direct Memory Access Server module
#include "tmra.h" ///< Timer module
#include "diom.h" ///< Digital Input Output module

#include "mdll.h" ///< Module List

#define MAIN_TI_ms_TASK_DELAY_U32 ((uint32_t) 5)