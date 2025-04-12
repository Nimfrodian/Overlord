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

        tTIMH_INITDATA_STR timh_cfgData_str =
        {
            .nr_moduleId_U32 = MODULE_TIMH,
            .timh_ti_us_sysTimeFunc_pfS64 = &tmra_ti_us_getCurrentTime_S64,
        };
        timh_init(&timh_cfgData_str);

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
        sera_print("SERA module initialized\n");

        tCANM_INITDATA_STR CanmCfg =
        {
            .nr_moduleId_U32 = MODULE_CANM,
        };
        canm_init(&CanmCfg);
        sera_print("CANM module initialized\n");

        tRTDB_INITDATA_STR RtdbCfg =
        {
            .nr_moduleId_U32 = MODULE_RTDB,
        };
        rtdb_init(&RtdbCfg);
        sera_print("RTDB module initialized\n");

        tDMAS_INITDATA_STR DmasCfg =
        {
            .nr_moduleId_U32 = MODULE_DMAS,
        };
        dmas_init(&DmasCfg);
        sera_print("DMAS module initialized\n");

        tDIOM_INITDATA_STR DiomCfg =
        {
            .nr_moduleId_U32 = MODULE_DIOM,
        };
        diom_init(&DiomCfg);
        sera_print("DIOM module initialized\n");

        sera_print("Initialization time: %lli us\n", timh_ti_us_readSystemTime_S64());
    }

    pina_setGpioAsOutput(PINA_LED_0);
    pina_setGpioAsOutput(PINA_LED_1);
    pina_setGpioAsOutput(PINA_LED_2);

    sera_print("Setting init complete LED ON\n");
    pina_setGpioLevel(PINA_LED_0, 1);
}