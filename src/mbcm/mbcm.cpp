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
        tU32 currRelayStates_U32 = 0; // TODO

        // cycle through all modbus modules and send command to update if neccessary
        // start with a different module each time
        tU8 desiredRelayState_U32 = (tU8) rtdb_read_tU32S(MBCM_X_DESIREDRELAYSTATES_U32);
        //if (currRelayStates_U32 != desiredRelayState_U32)
        {
            tU8 MbMsgData[10];
            MbMsgData[0] = 0x01;                    // ID
            MbMsgData[1] = 0x0F;                    // function code
            MbMsgData[2] = 0x00;                    // Relay start address
            MbMsgData[3] = 0x00;                    // Relay start address
            MbMsgData[4] = 0x00;                    // Number of relays
            MbMsgData[5] = 0x08;                    // Number of relays
            MbMsgData[6] = 0x01;                    // following bytes
            MbMsgData[7] = desiredRelayState_U32;   // bitmap of new relay statuses
            unsigned int crc = crcm_CRC16(MbMsgData, 8);
            MbMsgData[8] = (crc >> 8) & 0xFF;       // CRC HI
            MbMsgData[9] = (crc >> 0) & 0xFF;       // CRC LOW
            // send command to update relay states
            pina_setGpioLevel(PINA_MB_1_DE, 1); // set DE pin high
            uart_write_bytes(MBCM_MB1_UART_NUM_STR, (const char*) MbMsgData, 10);
            uart_wait_tx_done(MBCM_MB1_UART_NUM_STR, pdMS_TO_TICKS(1));
            pina_setGpioLevel(PINA_MB_1_DE, 0); // set DE pin low
            currRelayStates_U32 = desiredRelayState_U32;    // TODO: remove
        }
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
