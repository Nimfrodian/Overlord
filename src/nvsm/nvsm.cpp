#include "nvsm.h"
#include "errh.h"
#include "mdll.h"

static tB nvsm_s_moduleInit_tB = false;
static uint32_t nvsm_ti_ms_taskDelay_U32 = 0;

static nvs_handle_t nvsm_x_nvsHandle_STR;

static tB nvsm_s_commitFlag_tB = false;

void nvsm_init(tNVSM_INITDATA_STR* nvsmCfg)
{
    if (true == nvsm_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, MODULE_NVSM, 0, NVSM_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == nvsmCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, MODULE_NVSM, 0, NVSM_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        // Initialize NVS
        tU32 errFlashInit_U32 = nvs_flash_init();
        if (errFlashInit_U32 == ESP_ERR_NVS_NO_FREE_PAGES || errFlashInit_U32 == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            // NVS partition was truncated and needs to be erased
            // Retry nvs_flash_init
            tU32 errFlashErase = nvs_flash_erase();
            if (ESP_OK != errFlashErase)
            {
                errh_reportError(ERRH_ERROR_HIGH, MODULE_NVSM, errFlashErase, NVSM_API_INIT_U32, NVSM_ERR_CANNOT_ERASE_FLASH_U32);
            }
            else
            {
                // Retry nvs_flash_init
                errFlashInit_U32 = nvs_flash_init();
            }
        }
        if (ESP_OK != errFlashInit_U32)
        {
            errh_reportError(ERRH_ERROR_HIGH, MODULE_NVSM, errFlashInit_U32, NVSM_API_INIT_U32, NVSM_ERR_CANNOT_INIT_FLASH_U32);
        }
        else
        {
            tU32 errOpen = nvs_open("storage", NVS_READWRITE, &nvsm_x_nvsHandle_STR);

            if (ESP_OK != errOpen)
            {
                errh_reportError(ERRH_ERROR_HIGH, MODULE_NVSM, errFlashInit_U32, NVSM_API_INIT_U32, NVSM_ERR_CANNOT_OPEN_FLASH_U32);
            }
            else
            {
                nvsm_s_moduleInit_tB = true;
                nvsm_ti_ms_taskDelay_U32 = nvsmCfg->ti_ms_taskDelay_U32;
            }
        }
    }
}

tU32 nvsm_read_tU8S(const char* key_pU8, tU8* value_pU8)
{
    tU32 err_U32 = nvs_get_u8(nvsm_x_nvsHandle_STR, key_pU8, value_pU8);
    if (ESP_OK != err_U32)
    {
        errh_reportError(ERRH_WARNING, MODULE_NVSM, err_U32, NVSM_API_NVSM_READ_U8, NVSM_ERR_CANNOT_READ_U8);
    }
    return err_U32;
}

void nvsm_write_tU8S(const char* key_pU8, tU8 value_U8)
{
    tU32 err_U32 = nvs_set_u8(nvsm_x_nvsHandle_STR, key_pU8, value_U8);
    if (ESP_OK != err_U32)
    {
        errh_reportError(ERRH_WARNING, MODULE_NVSM, err_U32, NVSM_API_NVSM_WRITE_U8, NVSM_ERR_CANNOT_WRITE_U8);
    }
    else
    {
        nvsm_s_commitFlag_tB = true;
    }
}

tU32 nvsm_read_tU32S(const char* key_pU8, tU32* value_pU32)
{
    tU32 err_U32 = nvs_get_u32(nvsm_x_nvsHandle_STR, key_pU8, value_pU32);
    if (ESP_OK != err_U32)
    {
        errh_reportError(ERRH_WARNING, MODULE_NVSM, err_U32, NVSM_API_NVSM_READ_U32, NVSM_ERR_CANNOT_READ_U32);
    }
    return err_U32;
}

void nvsm_write_tU32S(const char* key_pU8, tU32 value_U32)
{
    tU32 err_U32 = nvs_set_u32(nvsm_x_nvsHandle_STR, key_pU8, value_U32);
    if (ESP_OK != err_U32)
    {
        errh_reportError(ERRH_WARNING, MODULE_NVSM, err_U32, NVSM_API_NVSM_WRITE_U32, NVSM_ERR_CANNOT_WRITE_U32);
    }
    else
    {
        nvsm_s_commitFlag_tB = true;
    }
}

void nvsm_run_5ms(void)
{
    static tU32 nvsmTaskCounter_U32 = 0;
    nvsmTaskCounter_U32 += nvsm_ti_ms_taskDelay_U32;
    if (nvsmTaskCounter_U32 >= 250)
    {
        if (true == nvsm_s_commitFlag_tB)
        {
            tU32 err_U32 = nvs_commit(nvsm_x_nvsHandle_STR);
            if (ESP_OK != err_U32)
            {
                errh_reportError(ERRH_WARNING, MODULE_NVSM, err_U32, NVSM_API_NVSM_RUN_U32, NVSM_ERR_CANNOT_WRITE_U32);
            }
            nvsm_s_commitFlag_tB = false;
        }
        nvsmTaskCounter_U32 = 0;
    }
}
