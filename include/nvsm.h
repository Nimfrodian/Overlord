/*
*   Non-volatile storage module
*   used to store information across resets
*
*/

#pragma once

#include "nvs.h"
#include "nvs_flash.h"
#include "vars.h"

#define NVSM_API_INIT_U32                  ((uint32_t) 1)
#define NVSM_API_NVSM_RUN_U32              ((uint32_t) 2)
#define NVSM_API_NVSM_READ_U32             ((uint32_t) 3)
#define NVSM_API_NVSM_WRITE_U32            ((uint32_t) 4)
#define NVSM_API_NVSM_READ_U8              ((uint32_t) 5)
#define NVSM_API_NVSM_WRITE_U8             ((uint32_t)64)

#define NVSM_ERR_CANNOT_ERASE_FLASH_U32    ((uint32_t) 1)
#define NVSM_ERR_CANNOT_INIT_FLASH_U32     ((uint32_t) 2)
#define NVSM_ERR_CANNOT_OPEN_FLASH_U32     ((uint32_t) 3)
#define NVSM_ERR_CANNOT_READ_U32           ((uint32_t) 4)
#define NVSM_ERR_CANNOT_WRITE_U32          ((uint32_t) 5)
#define NVSM_ERR_CANNOT_COMMIT_U32         ((uint32_t) 6)
#define NVSM_ERR_CANNOT_READ_U8            ((uint32_t) 7)
#define NVSM_ERR_CANNOT_WRITE_U8           ((uint32_t) 8)

typedef struct
{
    uint32_t ti_ms_taskDelay_U32;   ///< time in ms between task executions
} tNVSM_INITDATA_STR;

void nvsm_init(tNVSM_INITDATA_STR* nvsmCfg);
void nvsm_run_5ms(void);

tU32 nvsm_read_tU8S(const char* key_pU8, tU8* value_pU8);
void nvsm_write_tU8S(const char* key_pU8, tU8 value_U8);
tU32 nvsm_read_tU32S(const char* key_pU8, tU32* value_pU32);
void nvsm_write_tU32S(const char* key_pU8, tU32* value_pU32);