#include "main.h"

// Firmware version 2.xx.yy

extern "C" void app_main(void)
{
    {
        tTMRA_INITDATA_STR TmraCfg =
        {};
        tmra_init(&TmraCfg);

        tTIMH_INITDATA_STR timh_cfgData_str =
        {
            .timh_ti_us_sysTimeFunc_pfS64 = &tmra_ti_us_getCurrentTime_S64,
        };
        timh_init(&timh_cfgData_str);

        tERRH_INITDATA_STR ErrhCfg =
        {
            .ti_us_readSystemTimeCallback_S64 = tmra_ti_us_getCurrentTime_S64,
            .ti_readCurrentGlobalTimeCallback_S64 = timh_ti_readCurrentGlobalTime,
            .reportErrorCallback = ERRH_INTERNAL_CALLBACK,
        };
        errh_init(&ErrhCfg);

        tSERA_INITDATA_STR SeraCfg =
        {};
        sera_init(&SeraCfg);
        sera_print("SERA module initialized\n");

        tNVSM_INITDATA_STR NvsmCfg =
        {
            .ti_ms_taskDelay_U32 = MAIN_TI_ms_TASK_DELAY_U32, // executed in main task
        };
        nvsm_init(&NvsmCfg);
        sera_print("NVSM module initialized\n");

        tCANP_INITDATA_STR CanpCfg =
        {
            .ti_ms_taskDelay_U32 = MAIN_TI_ms_TASK_DELAY_U32, // executed in main task
        };
        canp_init(&CanpCfg);

        rtdp_init();
        sera_print("RTDB module initialized\n");

        tDMAS_INITDATA_STR DmasCfg =
        {};
        dmas_init(&DmasCfg);
        sera_print("DMAS module initialized\n");

        tDIOM_INITDATA_STR DiomCfg =
        {};
        diom_init(&DiomCfg);
        sera_print("DIOM module initialized\n");

        tMBCM_INITDATA_STR MbcmCfg =
        {};
        mbcm_init(&MbcmCfg);
        sera_print("MBCM module initialized\n");

        tDIMH_INITDATA_STR DimhCfg =
        {};
        dimh_init(&DimhCfg);
        sera_print("DIMH module initialized\n");

        sera_print("Initialization time: %lli us\n", timh_ti_us_readSystemTime_S64());
    }

    pina_setGpioAsOutput(PINA_LED_0);
    pina_setGpioAsOutput(PINA_LED_1);
    pina_setGpioAsOutput(PINA_LED_2);

    sera_print("Setting init complete LED ON\n");
    pina_setGpioLevel(PINA_LED_0, 1);

    // Main loop
    while(1)
    {
        {
            tCANM_X_CANMSGDATA_STR* rxDataPtr = canm_pstr_readCanMsgData(CAN_DMAS_COMMAND_MESSAGE);
            if (1 == rxDataPtr->canRdyForParse_tB)
            {
                dmas_canMsgParse_ev( rxDataPtr->canMsg_str.data, &rxDataPtr->canMsg_str.identifier);
                canm_x_clearFlagForCanMsgParse(CAN_DMAS_COMMAND_MESSAGE);
            }
        }

        {
            for (tU8 dmasMsgIndx = CAN_DMAS_SEND_MESSAGE0; dmasMsgIndx <= CAN_DMAS_SEND_MESSAGE15; dmasMsgIndx++)
            {
                tDMAS_MESSAGEDATA_STR preparedData = dmas_getReadyData();
                if (true == preparedData.isReady_U8)
                {
                    tCANM_X_CANMSGDATA_STR* msgPtr_pstr = canm_pstr_readCanMsgData((tCANM_CANMSGINDX_E) dmasMsgIndx);
                    msgPtr_pstr->canMsg_str.data[0] = preparedData.payload_U8[0];
                    msgPtr_pstr->canMsg_str.data[1] = preparedData.payload_U8[1];
                    msgPtr_pstr->canMsg_str.data[2] = preparedData.payload_U8[2];
                    msgPtr_pstr->canMsg_str.data[3] = preparedData.payload_U8[3];
                    msgPtr_pstr->canMsg_str.data[4] = preparedData.payload_U8[4];
                    msgPtr_pstr->canMsg_str.data[5] = preparedData.payload_U8[5];
                    msgPtr_pstr->canMsg_str.data[6] = preparedData.payload_U8[6];
                    msgPtr_pstr->canMsg_str.data[7] = preparedData.payload_U8[7];

                    msgPtr_pstr->canMsg_str.identifier = 0x20;
                    msgPtr_pstr->canRdyForTx_tB = true;
                }
                else
                {
                    break;
                }
            }
        }

        // 100ms tasks
        static uint32_t counter = 0;
        if (counter > 100)
        {
            counter = 0;
            tCANM_X_CANMSGDATA_STR* errh_canMsg0_pstr = canm_pstr_readCanMsgData(CAN_ERROR_MESSAGE0);
            tCANM_X_CANMSGDATA_STR* errh_canMsg1_pstr = canm_pstr_readCanMsgData(CAN_ERROR_MESSAGE1);
            tCANM_X_CANMSGDATA_STR* errh_canMsg2_pstr = canm_pstr_readCanMsgData(CAN_ERROR_MESSAGE2);
            tCANM_X_CANMSGDATA_STR* errh_canMsg3_pstr = canm_pstr_readCanMsgData(CAN_ERROR_MESSAGE3);

            errh_canMsg0_pstr->canMsg_str.identifier = 0x50;
            errh_canMsg1_pstr->canMsg_str.identifier = 0x51;
            errh_canMsg2_pstr->canMsg_str.identifier = 0x52;
            errh_canMsg3_pstr->canMsg_str.identifier = 0x53;

            uint8_t* canData_aU8[4] = {errh_canMsg0_pstr->canMsg_str.data,
                                    errh_canMsg1_pstr->canMsg_str.data,
                                    errh_canMsg2_pstr->canMsg_str.data,
                                    errh_canMsg3_pstr->canMsg_str.data,
                                    };

            if (true == errh_canMsgCompose_100ms(canData_aU8))
            {
                canm_x_flagCanMsgForTx(CAN_ERROR_MESSAGE0);
                canm_x_flagCanMsgForTx(CAN_ERROR_MESSAGE1);
                canm_x_flagCanMsgForTx(CAN_ERROR_MESSAGE2);
                canm_x_flagCanMsgForTx(CAN_ERROR_MESSAGE3);
            }
        }
        counter += MAIN_TI_ms_TASK_DELAY_U32;

        canm_transceive_run_5ms();

        dmas_run_5ms();

        diom_run_5ms();

        nvsm_run_5ms();

        dimh_run_5ms();

        vTaskDelay(MAIN_TI_ms_TASK_DELAY_U32 / portTICK_PERIOD_MS);
    }
}