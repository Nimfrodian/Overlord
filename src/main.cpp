#include "main.h"

// Firmware version 2.xx.yy

extern "C" void app_main(void)
{
    {
        tTMRA_INITDATA_STR TmraCfg =
        {
            .nr_moduleId_U32 = MODULE_TMRA,
        };
        tmra_init(&TmraCfg);

        tERRH_INITDATA_STR ErrhCfg =
        {
            .nr_moduleId_U32 = MODULE_ERRH,
        };
        errh_init(&ErrhCfg);

        tSERA_INITDATA_STR SeraCfg =
        {
            .nr_moduleId_U32 = MODULE_SERA,
        };
        sera_init(&SeraCfg);
        sera_print("Sera module initialized\n");

        tCANM_INITDATA_STR CanmCfg =
        {
            .nr_moduleId_U32 = MODULE_CANM,
        };
        canm_init(&CanmCfg);
        sera_print("Canm module initialized\n");

        tRTDB_INITDATA_STR RtdbCfg =
        {
            .nr_moduleId_U32 = MODULE_RTDB,
        };
        rtdb_init(&RtdbCfg);
        sera_print("Rtdb module initialized\n");

        tDMAS_INITDATA_STR DmasCfg =
        {
            .nr_moduleId_U32 = MODULE_DMAS,
        };
        dmas_init(&DmasCfg);
        sera_print("DMAS module initialized\n");

        sera_print("Initialization time: %lli us\n", timh_ti_us_readSystemTime_S64());
    }

    gpio_reset_pin(GPIO_NUM_13);
    gpio_reset_pin(GPIO_NUM_14);
    gpio_reset_pin(GPIO_NUM_21);
    gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_14, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_21, GPIO_MODE_OUTPUT);

    sera_print("Setting init complete LED ON\n");
    gpio_set_level(GPIO_NUM_13, 1);
}