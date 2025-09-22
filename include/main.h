#pragma once

#include "espa.h"
#include "stdint.h"
#include "string.h"

#include "canp.h"
#include "rtdb.h" ///< Real Time Database module
#include "dmas.h" ///< Direct Memory Access Server module
#include "timh.h" ///< Time Handling module
#include "tmra.h" ///< Timer module
#include "diom.h" ///< Digital Input Output module
#include "mbcm.h" ///< Modbus Control module
#include "nvsm.h" ///< Non-volatile Storage module
#include "dimh.h" ///< Dimmer Handler module
#include "rtdp.h"

#include "mdll.h" ///< Module List

#define MAIN_TI_ms_TASK_DELAY_U32 ((uint32_t) 5)