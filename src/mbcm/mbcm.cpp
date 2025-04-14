#include "mbcm.h"
#include "rtdb.h"
#include "tmra.h"
#include "pina.h"
#include "crcm.h"
#include "mbcm_rtdb.h"

static uint32_t mbcm_nr_moduleId_U32 = 0;
static bool mbcm_s_moduleInit_tB = false;

void Modbus1_task(void* param)
{
    while (1)
    {
        // check UART input buffer and parse data
        static tU8 currRelayStates_aU8[MBCM_MAX_RELAY_BOARDS_U32] = {};
        static tU8 lastSentState_U8 = 0;
        tU8 buffer[32]= {0};
        volatile tU8 length_U8 = 0;
        length_U8 = uart_read_bytes(MBCM_MB1_UART_NUM_STR, buffer, 100, 0);
        if (length_U8)
        {
            if (0x08 == buffer[5])   // check if written relays is 8 - it means all relays were written to
            {
                tU8 boardIndex_U8 = buffer[0] - 1;
                if ((boardIndex_U8 < MBCM_MAX_RELAY_BOARDS_U32) && (boardIndex_U8 >= 0))
                {
                    currRelayStates_aU8[boardIndex_U8] = lastSentState_U8;
                }
                else
                {
                    errh_reportError(ERRH_ERROR_CRITICAL, mbcm_nr_moduleId_U32, boardIndex_U8, MBCM_API_MB1_RUN_U32, ERRH_ERR_READ_INDEX_OUT_OF_BOUNDS_U32);
                }
            }
        }
        uart_flush_input(MBCM_MB1_UART_NUM_STR);

        // cycle through all modbus modules and send command to update if neccessary. Start with a different module each time
        // TODO: add relay invert
        // First four IDs are reserved for GPIO controlled relay boards
        rtdb_write_tU8S(MBCM_X_DESIREDRELAYSTATES_AU8_0, (rtdb_read_tU32S(DIOM_X_INPUTSTATES_U32) >>  0) & 0xFF);
        rtdb_write_tU8S(MBCM_X_DESIREDRELAYSTATES_AU8_1, (rtdb_read_tU32S(DIOM_X_INPUTSTATES_U32) >>  8) & 0xFF);
        rtdb_write_tU8S(MBCM_X_DESIREDRELAYSTATES_AU8_2, (rtdb_read_tU32S(DIOM_X_INPUTSTATES_U32) >> 16) & 0xFF);
        rtdb_write_tU8S(MBCM_X_DESIREDRELAYSTATES_AU8_3, (rtdb_read_tU32S(DIOM_X_INPUTSTATES_U32) >> 24) & 0xFF);

        static tU8 prevI = 0;
        tU8 currI = 0;
        for (tU8 i = 0; i < MBCM_MAX_RELAY_BOARDS_U32; i++)
        {
            currI = prevI + i;
            if (currI >= MBCM_MAX_RELAY_BOARDS_U32) currI -= MBCM_MAX_RELAY_BOARDS_U32; // wrap around

            tU8 currRelayState_U8 = currRelayStates_aU8[currI];   // get current relay state
            tU8 desiredRelayState_U8 = rtdb_read_tU8S((tU8SEnumT)(MBCM_X_DESIREDRELAYSTATES_AU8_0 + currI));   // get desired relay state
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
                unsigned int crc = crcm_CRC16(MbMsgData, 8);
                MbMsgData[8] = (crc >> 8) & 0xFF;       // CRC HI
                MbMsgData[9] = (crc >> 0) & 0xFF;       // CRC LOW
                // send command to update relay states
                pina_setGpioLevel(PINA_MB_1_DE, 1); // set DE pin high
                uart_write_bytes(MBCM_MB1_UART_NUM_STR, (const char*) MbMsgData, 10);
                uart_wait_tx_done(MBCM_MB1_UART_NUM_STR, pdMS_TO_TICKS(1));
                pina_setGpioLevel(PINA_MB_1_DE, 0); // set DE pin low
                break;
            }
        }
        prevI = currI + 1; // update previous index
        if (prevI >= MBCM_MAX_RELAY_BOARDS_U32) prevI -= MBCM_MAX_RELAY_BOARDS_U32; // wrap around
        vTaskDelay(pdMS_TO_TICKS(rtdb_read_tU32S(MBCM_TI_US_TASKPERIODRELAY_U32) / 1000));
    }
    // delete task if illegal state was reached
    vTaskDelete( NULL );
}

void Modbus2_task(void* param)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(rtdb_read_tU32S(MBCM_TI_US_TASKPERIODPWRMETER_U32) / 1000));
    }
    // delete task if illegal state was reached
    vTaskDelete( NULL );
}

void mbcm_init(tMBCM_INITDATA_STR* mbcmCfg)
{
    if (true == mbcm_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, mbcm_nr_moduleId_U32, 0, MBCM_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == mbcmCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, mbcm_nr_moduleId_U32, 0, MBCM_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        mbcm_nr_moduleId_U32 = mbcmCfg->nr_moduleId_U32;
        mbcm_s_moduleInit_tB = true;    // only init once

        mbcm_rtdb_init();    // initialize RTDB

        pina_setGpioAsOutput(PINA_MB_1_DE);
        pina_setGpioAsOutput(PINA_MB_2_DE);

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
        uart_set_pin(MBCM_MB1_UART_NUM_STR, PINA_MB_1_TX, PINA_MB_1_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        uart_param_config(MBCM_MB1_UART_NUM_STR, &uart1_config);
        uart_driver_install(MBCM_MB1_UART_NUM_STR, 1024, 0, 0, NULL, 0);
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
        uart_set_pin(MBCM_MB2_UART_NUM_STR, PINA_MB_2_TX, PINA_MB_2_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        uart_param_config(MBCM_MB2_UART_NUM_STR, &uart2_config);
        uart_driver_install(MBCM_MB2_UART_NUM_STR, 1024, 0, 0, NULL, 0);
        uart_set_mode(MBCM_MB2_UART_NUM_STR, UART_MODE_RS485_HALF_DUPLEX);

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
}
