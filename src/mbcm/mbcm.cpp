#include "mbcm.h"
#include "rtdb.h"
#include "tmra.h"
#include "pina.h"
#include "crcm.h"
#include "nvsm.h"
#include "mdll.h"
#include "mbcm_rtdb.h"

static bool mbcm_s_moduleInit_tB = false;
static tU8 currRelayStates_aU8[MBCM_MAX_RELAY_BOARDS_U32] = {};

static const char* mbcm_x_nvsKey_aSTR[MBCM_MAX_RELAY_BOARDS_U32] =
{
    "INV_REL_ST_0",     // Invert relay state 0
    "INV_REL_ST_1",     // Invert relay state 1
    "INV_REL_ST_2",     // Invert relay state 2
    "INV_REL_ST_3",     // Invert relay state 3
    "INV_REL_ST_4",     // Invert relay state 4
    "INV_REL_ST_5",     // Invert relay state 5
    "INV_REL_ST_6",     // Invert relay state 6
    "INV_REL_ST_7",     // Invert relay state 7
    "INV_REL_ST_8",     // Invert relay state 8
    "INV_REL_ST_9",     // Invert relay state 9
    "INV_REL_ST_10",    // Invert relay state 10
    "INV_REL_ST_11",    // Invert relay state 11
    "INV_REL_ST_12",    // Invert relay state 12
    "INV_REL_ST_13",    // Invert relay state 13
    "INV_REL_ST_14",    // Invert relay state 14
    "INV_REL_ST_15",    // Invert relay state 15
};

void Modbus1_task(void* param)
{
    while (1)
    {
        tU32 taskPeriod_U32 = rtdb_read_tU32S(RTDB_MBCM_TI_US_TASKPERIODRELAY_U32);
        // check UART input buffer and parse data
        static tU8 lastSentState_U8 = 0;
        tU8 buffer[32]= {0};
        tU8 length_U8 = 0;
        length_U8 = uart_read_bytes(MBCM_MB1_UART_NUM_STR, buffer, 32, 0);
        if (length_U8)
        {
            if (0x08 == buffer[5])   // check if written relays is 8 - it means all relays were written to
            {
                tU8 boardIndex_U8 = buffer[0] - 1;
                if (boardIndex_U8 < MBCM_MAX_RELAY_BOARDS_U32)
                {
                    currRelayStates_aU8[boardIndex_U8] = lastSentState_U8;
                    rtdb_write_tU8S((tU8SEnumT)(RTDB_MBCM_X_ACTUALRELAYSTATES_AU8_0 + boardIndex_U8), lastSentState_U8);
                }
                else
                {
                    errh_reportError(ERRH_ERROR_CRITICAL, MODULE_MBCM, boardIndex_U8, MBCM_API_MB1_RUN_U32, ERRH_ERR_READ_INDEX_OUT_OF_BOUNDS_U32);
                }
            }
            else
            {
                errh_reportError(ERRH_NOTIF, MODULE_MBCM, 0, MBCM_API_MB1_RUN_U32, MBCM_ERR_WRONG_DATA_U32);
            }
        }
        uart_flush_input(MBCM_MB1_UART_NUM_STR);

        tU8 gpioState[4] = {};
        gpioState[0] = (rtdb_read_tU32S(RTDB_DIOM_X_INPUTSTATES_U32) >>  0) & 0xFF;
        gpioState[1] = (rtdb_read_tU32S(RTDB_DIOM_X_INPUTSTATES_U32) >>  8) & 0xFF;
        gpioState[2] = (rtdb_read_tU32S(RTDB_DIOM_X_INPUTSTATES_U32) >> 16) & 0xFF;
        gpioState[3] = (rtdb_read_tU32S(RTDB_DIOM_X_INPUTSTATES_U32) >> 24) & 0xFF;

        static tU8 prevInvReq_U8[MBCM_MAX_RELAY_BOARDS_U32] = {};
        tU8 currInvertReq_U8[MBCM_MAX_RELAY_BOARDS_U32] = {};
        for (tU8 i = 0; i < MBCM_MAX_RELAY_BOARDS_U32; i++)
        {
            for (tU8 j = 0; j < 8; j++)
            {
                currInvertReq_U8[i] |= (rtdb_read_tBS((tBSEnumT)(RTDB_CANM_S_RXRELAYINVERTREQ_AB_0 + i*8 + j))) << j;
            }
            if (currInvertReq_U8[i] != prevInvReq_U8[i])
            {
                nvsm_write_tU8S(mbcm_x_nvsKey_aSTR[i], currInvertReq_U8[i]);   // write to NVS
                prevInvReq_U8[i] = currInvertReq_U8[i];
            }

            if (i < 4)
            {
                rtdb_write_tU8S((tU8SEnumT)(RTDB_MBCM_X_DESIREDRELAYSTATES_AU8_0 + i), currInvertReq_U8[i] ^ gpioState[i]);
            }
            else
            {
                rtdb_write_tU8S((tU8SEnumT)(RTDB_MBCM_X_DESIREDRELAYSTATES_AU8_0 + i), currInvertReq_U8[i]);
            }
        }

        static tU8 prevI = 0;
        tU8 currI = 0;
        for (tU8 i = 0; i < MBCM_MAX_RELAY_BOARDS_U32; i++)
        {
            currI = prevI + i;
            if (currI >= MBCM_MAX_RELAY_BOARDS_U32) currI -= MBCM_MAX_RELAY_BOARDS_U32; // wrap around

            tU8 currRelayState_U8 = currRelayStates_aU8[currI];   // get current relay state
            tU8 desiredRelayState_U8 = rtdb_read_tU8S((tU8SEnumT)(RTDB_MBCM_X_DESIREDRELAYSTATES_AU8_0 + currI));   // get desired relay state
            if (currRelayState_U8 != desiredRelayState_U8)
            {
                tU8 MbMsgData[10];
                MbMsgData[0] = currI + 1;               // ID
                MbMsgData[1] = 0x0F;                    // function code
                MbMsgData[2] = 0x00;                    // Relay start address
                MbMsgData[3] = 0x00;                    // Relay start address
                MbMsgData[4] = 0x00;                    // Number of relays
                MbMsgData[5] = 0x08;                    // Number of relays
                MbMsgData[6] = 0x01;                    // following bytes
                MbMsgData[7] = desiredRelayState_U8;    // bitmap of new relay statuses
                lastSentState_U8 = MbMsgData[7];
                unsigned int crc = crcm_CRC16_Modbus(MbMsgData, 8);
                MbMsgData[8] = (crc >> 8) & 0xFF;       // CRC HI
                MbMsgData[9] = (crc >> 0) & 0xFF;       // CRC LOW
                // send command to update relay states
                uart_write_bytes(MBCM_MB1_UART_NUM_STR, (const char*) MbMsgData, 10);
                break;
            }
        }
        prevI = currI + 1; // update previous index
        if (prevI >= MBCM_MAX_RELAY_BOARDS_U32) prevI -= MBCM_MAX_RELAY_BOARDS_U32; // wrap around
        vTaskDelay(pdMS_TO_TICKS(taskPeriod_U32 / 1000));
    }
    // delete task if illegal state was reached
    vTaskDelete( NULL );
}

void Modbus2_task(void* param)
{
    while (1)
    {
        static tU8 varIndx_U8 = 0; // current variable index, 0-20
        static tU8 boardIndx_U8 = 0; // current board index, 0-20
        // check UART input buffer and parse data
        tU8 buffer[32]= {0};
        tU8 length_U8 = 0;
        length_U8 = uart_read_bytes(MBCM_MB2_UART_NUM_STR, buffer, 32, 0);
        if (length_U8)
        {
            if ((buffer[0] - 1) == boardIndx_U8)
            {
                uint32_t temp = ((uint32_t)buffer[3] << 24) |
                ((uint32_t)buffer[4] << 16) |
                ((uint32_t)buffer[5] << 8)  |
                ((uint32_t)buffer[6] << 0);
                float result = *((float *)&temp); // convert uint32_t value to float variable
                rtdb_write_tF32S((tF32SEnumT)(RTDB_MBCM_X_SDM120MREADINGS_0_AF32_0 + boardIndx_U8 * 21 + varIndx_U8), result); // store the result
            }
            else
            {
                rtdb_setSS_tF32S((tF32SEnumT)(RTDB_MBCM_X_SDM120MREADINGS_0_AF32_0 + boardIndx_U8 * 21 + varIndx_U8), SIGNAL_MISSING);
                errh_reportError(ERRH_NOTIF, MODULE_MBCM, 0, MBCM_API_MB2_RUN_U32, MBCM_ERR_WRONG_DATA_U32);
            }
        }
        else
        {
            rtdb_setSS_tF32S((tF32SEnumT)(RTDB_MBCM_X_SDM120MREADINGS_0_AF32_0 + boardIndx_U8 * 21 + varIndx_U8), SIGNAL_MISSING);
        }

        varIndx_U8++;
        if (varIndx_U8 >= SDM120M_NUM_OF_READ)
        {
            varIndx_U8 = 0; // reset index
            boardIndx_U8++;
            if (boardIndx_U8 >= MBCM_MAX_PWR_METERS_U32)
            {
                boardIndx_U8 = 0;
            }
        }
        uart_flush_input(MBCM_MB2_UART_NUM_STR);

        // write next request
        tU8 MbMsgData[8];
        MbMsgData[0] = boardIndx_U8 + 1;        // ID
        MbMsgData[1] = 0x04;                    // function code
        MbMsgData[2] = (mbcm_sdm120mStartAddrs_aU16[varIndx_U8] >> 8) & 0xFF;   // Data Reg High HI
        MbMsgData[3] = (mbcm_sdm120mStartAddrs_aU16[varIndx_U8] >> 0) & 0xFF;   // Data Reg High LO
        MbMsgData[4] = 0x00;                    // Number of relays
        MbMsgData[5] = 0x02;                    // Number of relays
        unsigned int crc = crcm_CRC16_Modbus(MbMsgData, 6);
        MbMsgData[6] = (crc >> 8) & 0xFF;       // CRC HI
        MbMsgData[7] = (crc >> 0) & 0xFF;       // CRC LOW
        // send command to read state
        uart_write_bytes(MBCM_MB2_UART_NUM_STR, (const char*) MbMsgData, 8);

        vTaskDelay(pdMS_TO_TICKS(rtdb_read_tU32S(RTDB_MBCM_TI_US_TASKPERIODPWRMETER_U32) / 1000));
    }
    // delete task if illegal state was reached
    vTaskDelete( NULL );
}

void Modbus_prerunTask(void* param)
{
    for (tU8 rel_indx = 0; rel_indx < MBCM_MAX_RELAY_BOARDS_U32; rel_indx++)
    {
        tU8 MbMsgData[8];
        MbMsgData[0] = rel_indx + 1;            // ID
        MbMsgData[1] = 0x01;                    // function code
        MbMsgData[2] = 0x00;                    // Relay start address
        MbMsgData[3] = 0x00;                    // Relay start address
        MbMsgData[4] = 0x00;                    // Number of relays
        MbMsgData[5] = 0x08;                    // Number of relays
        unsigned int crc = crcm_CRC16_Modbus(MbMsgData, 8);
        MbMsgData[6] = (crc >> 8) & 0xFF;       // CRC HI
        MbMsgData[7] = (crc >> 0) & 0xFF;       // CRC LOW
        // send command to update relay states
        uart_write_bytes(MBCM_MB1_UART_NUM_STR, (const char*) MbMsgData, 8);
        vTaskDelay(pdMS_TO_TICKS(5));

        // Read current relay states
        tU8 buffer[32]= {0};
        tU8 length_U8 = 0;
        length_U8 = uart_read_bytes(MBCM_MB1_UART_NUM_STR, buffer, 32, 0);
        if (length_U8)
        {
            if (0x01 == buffer[2])   // check if written relays is 8 - it means all relays were written to
            {
                tU8 boardIndex_U8 = buffer[0] - 1;
                if (boardIndex_U8 < MBCM_MAX_RELAY_BOARDS_U32)
                {
                    currRelayStates_aU8[boardIndex_U8] = buffer[3];
                    rtdb_write_tU8S((tU8SEnumT)(RTDB_MBCM_X_ACTUALRELAYSTATES_AU8_0 + boardIndex_U8), currRelayStates_aU8[boardIndex_U8]);
                }
                else
                {
                    errh_reportError(ERRH_ERROR_CRITICAL, MODULE_MBCM, boardIndex_U8, MBCM_API_MB_PRERUN_U32, ERRH_ERR_READ_INDEX_OUT_OF_BOUNDS_U32);
                }
            }
            else
            {
                errh_reportError(ERRH_NOTIF, MODULE_MBCM, 0, MBCM_API_MB_PRERUN_U32, MBCM_ERR_WRONG_DATA_U32);
            }
        }
        uart_flush_input(MBCM_MB1_UART_NUM_STR);
    }

    {
        // Start the command tasks
        xTaskCreatePinnedToCore(
        Modbus1_task,    // Function that should be called
        "Modbus1_task",   // Name of the task (for debugging)
        8192,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        NULL,            // Task handle
        1                // run on core 1
        );

        xTaskCreatePinnedToCore(
        Modbus2_task,    // Function that should be called
        "Modbus2_task",   // Name of the task (for debugging)
        8192,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        NULL,            // Task handle
        1                // run on core 1
        );
    }
    vTaskDelete( NULL ); // task completed
}

static void mbcm_updateDesiredRelayState(void)
{
    for (tU8 i = 0; i < MBCM_MAX_RELAY_BOARDS_U32; i++)
    {
        tU8 invertRelayState_U8 = 0;
        tU32 error_U32 = nvsm_read_tU8S(mbcm_x_nvsKey_aSTR[i], &invertRelayState_U8);
        if (ESP_ERR_NVS_NOT_FOUND == error_U32)
        {
            nvsm_write_tU8S(mbcm_x_nvsKey_aSTR[i], 0);
        }
        else if (ESP_OK != error_U32)
        {
            errh_reportError(ERRH_WARNING, MODULE_MBCM, error_U32, MBCM_API_INIT_U32, NVSM_ERR_CANNOT_READ_U8);
        }

        for (tU8 j = 0; j < 8; j++)
        {
            rtdb_write_tBS((tBSEnumT) (RTDB_CANM_S_RXRELAYINVERTREQ_AB_0 + i*8 + j), (invertRelayState_U8 >> j) & 0x01);
        }
    }
}

void mbcm_init(tMBCM_INITDATA_STR* mbcmCfg)
{
    if (true == mbcm_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, MODULE_MBCM, 0, MBCM_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == mbcmCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, MODULE_MBCM, 0, MBCM_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        mbcm_s_moduleInit_tB = true;    // only init once

        mbcm_updateDesiredRelayState();

        // Set Modbus 1 UART parameters
        uart_config_t uart1_config = {
            .baud_rate = MBCM_MB1_BAUDRATE_U32,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 122,
            .source_clk = UART_SCLK_APB,
            .flags = {
                .backup_before_sleep = 0,
            }
        };
        uart_driver_install(MBCM_MB1_UART_NUM_STR, 1024, 0, 0, NULL, 0);
        uart_param_config(MBCM_MB1_UART_NUM_STR, &uart1_config);
        uart_set_pin(MBCM_MB1_UART_NUM_STR, PINA_MB_1_TX, PINA_MB_1_RX, PINA_MB_1_DE, UART_PIN_NO_CHANGE);
        uart_set_mode(MBCM_MB1_UART_NUM_STR, UART_MODE_RS485_HALF_DUPLEX);

        // Set Modbus 2 UART parameters
        uart_config_t uart2_config = {
            .baud_rate = MBCM_MB2_BAUDRATE_U32,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 122,
            .source_clk = UART_SCLK_APB,
            .flags = {
                .backup_before_sleep = 0,
            }
        };
        uart_driver_install(MBCM_MB2_UART_NUM_STR, 1024, 0, 0, NULL, 0);
        uart_param_config(MBCM_MB2_UART_NUM_STR, &uart2_config);
        uart_set_pin(MBCM_MB2_UART_NUM_STR, PINA_MB_2_TX, PINA_MB_2_RX, PINA_MB_2_DE, UART_PIN_NO_CHANGE);
        uart_set_mode(MBCM_MB2_UART_NUM_STR, UART_MODE_RS485_HALF_DUPLEX);

        uart_flush_input(MBCM_MB1_UART_NUM_STR);
        uart_flush_input(MBCM_MB2_UART_NUM_STR);

        xTaskCreatePinnedToCore(
        Modbus_prerunTask,    // Function that should be called
        "Modbus_prerunTask",   // Name of the task (for debugging)
        8192,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        NULL,            // Task handle
        1                // run on core 1
        );
    }
}