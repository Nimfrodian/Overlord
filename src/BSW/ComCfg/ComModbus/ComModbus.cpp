#include "ComModbus.h"
#include "Dio_Cfg.h"    // used for debug LED
#include "Cli.h"

#define UART_MODBUS_1   UART_NUM_1
#define UART_MODBUS_2   UART_NUM_2

typedef enum
{
    COM_MODBUS_TRANSMITTING = 0,
    COM_MODBUS_RECEIVING,
} ComModbus_transcieveStType;

static ComModbus_transcieveStType _mb2TrnscieveType = COM_MODBUS_TRANSMITTING;

void ComModbus_init(void)
{
    CLI_INFO_0NL("Starting ComModbus_init()");

    // set DE and RE pins for MAX485 module
    gpio_reset_pin(DE_RE_PIN_MB1);
    gpio_reset_pin(DE_RE_PIN_MB2);
    CLI_INFO_1("Setting DE-RE for MB1 ... ");
    (ESP_OK == gpio_set_direction(DE_RE_PIN_MB1, GPIO_MODE_OUTPUT)) ? CLI_INFO_0NL("OK") : CLI_INFO_0NL("FAIL");
    CLI_INFO_1("Setting DE-RE for MB2 ... ");
    (ESP_OK == gpio_set_direction(DE_RE_PIN_MB2, GPIO_MODE_OUTPUT)) ? CLI_INFO_0NL("OK") : CLI_INFO_0NL("FAIL");

    ComCfg_init();

    // Configure MODBUS 2
    {
        CLI_INFO_0NL("Configuring Modbus 2");
        uart_config_t uart2_config = {
                .baud_rate = COM_MODBUS_2_BAUDRATE,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                .rx_flow_ctrl_thresh = 122,
                .source_clk = UART_SCLK_APB,
        };
        // Configure MODBUS 2 pins
        (ESP_OK == uart_set_pin(UART_MODBUS_2, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE))?
            CLI_INFO_1NL("uart_set_pin success") : CLI_INFO_1NL("uart_set_pin fail");
        // Set MODBUS 2 parameters
        (ESP_OK == uart_param_config(UART_MODBUS_2, &uart2_config))?
            CLI_INFO_1NL("uart_param_config success") : CLI_INFO_1NL("uart_param_config fail");
        // Install MODBUS 2 driver
        (ESP_OK == uart_driver_install(UART_MODBUS_2, 1024, 1024, 0, NULL, 0))?
            CLI_INFO_1NL("uart_driver_install success") : CLI_INFO_1NL("uart_driver_install fail");

        CLI_INFO_1NL("Creating Modbus 2 transcieve task ...");
        // create transceive task
        xTaskCreatePinnedToCore(
        ComModbus_2_transceive,    // Function that should be called
        "ComModbus_2_transceive",   // Name of the task (for debugging)
        8192,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        NULL,            // Task handle
        1                // run on core 1
        );
        CLI_INFO_1NL("Modbus 2 config DONE");
    }

    // Configure MODBUS 1
    {
        CLI_INFO_0NL("Configuring Modbus 1");
        uart_config_t uart1_config = {
                .baud_rate = COM_MODBUS_1_BAUDRATE,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                .rx_flow_ctrl_thresh = 122,
                .source_clk = UART_SCLK_APB,
        };
        // Configure MODBUS 1 pins
        {
            gpio_reset_pin(GPIO_NUM_18);
            gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
            gpio_reset_pin(GPIO_NUM_19);
            gpio_set_direction(GPIO_NUM_19, GPIO_MODE_INPUT);
        }
        (ESP_OK == uart_set_pin(UART_MODBUS_1, GPIO_NUM_18, GPIO_NUM_19, DE_RE_PIN_MB1, UART_PIN_NO_CHANGE))?
            CLI_INFO_1NL("uart_set_pin success") : CLI_INFO_1NL("uart_set_pin fail");
        // Set MODBUS 1 parameters
        (ESP_OK == uart_param_config(UART_MODBUS_1, &uart1_config))?
            CLI_INFO_1NL("uart_param_config success") : CLI_INFO_1NL("uart_param_config fail");
        // Install MODBUS 1 driver
        (ESP_OK == uart_driver_install(UART_MODBUS_1, 1024, 1024, 0, NULL, 0))?
            CLI_INFO_1NL("uart_driver_install success") : CLI_INFO_1NL("uart_driver_install fail");
        // Set RS485 half duplex mode
        (ESP_OK == uart_set_mode(UART_MODBUS_1, UART_MODE_RS485_HALF_DUPLEX))?
            CLI_INFO_1NL("uart_set_mode success") : CLI_INFO_1NL("uart_set_mode fail");

        CLI_INFO_1NL("Creating Modbus 1 transcieve task ...");
        // create transceive task
        xTaskCreatePinnedToCore(
        ComModbus_1_transceive,    // Function that should be called
        "ComModbus_1_transceive",   // Name of the task (for debugging)
        8192,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        NULL,            // Task handle
        1                // run on core 1
        );
        CLI_INFO_1NL("Modbus 1 config DONE");
    }
}

void ComModbus_1_transceive(void* param)
{
    while (1)
    {
        static uint16_t mbMsgIndx = 0;
        {
            // get message pointer
            ComCfg_Modbus1MsgDataType* txMsgPtr = ComCfg_read_mb1Config((ComCfg_modbus0MsgIndxType) mbMsgIndx);

            if (1 == txMsgPtr->mbRdyForTx)
            {
                // write the data to UART line
                uart_write_bytes(UART_MODBUS_1, (const char*) txMsgPtr->dataOut, txMsgPtr->dataOutCount);

                uart_wait_tx_done(UART_MODBUS_1, pdMS_TO_TICKS(5));

                // erase anything that is stuck in MODBUS 1 RX
                uart_flush_input(UART_MODBUS_1);

                // no need for response parse, simply flag as sent
                {
                    txMsgPtr->mbRdyForTx = 0;
                    txMsgPtr->mbRdyForParse = 1;
                }
            }
        }

        if (mbMsgIndx <= NUM_OF_MODBUS_1_MSG)
        {
            mbMsgIndx++;
        }
        else
        {
            mbMsgIndx = 0;
        }

        uint8_t buffer[100]= {0};
        uint8_t length = uart_read_bytes(UART_MODBUS_1, buffer, 100, pdMS_TO_TICKS(100));
        uart_write_bytes(UART_MODBUS_1, (const char*) buffer, length);

        vTaskDelay(pdMS_TO_TICKS(COM_MODBUS1_TASK_DELAY_TIME_MS));
    }


    // delete task if illegal state was reached
    vTaskDelete( NULL );
}

/**
 * @brief Function is used to copy the data from ModWriteBuffer into the premade Modbus buffer and flagging the message for TX (For Modbus 0)
 * @param MsgIndx Message index for the array of Modbus messages
 * @param ModWriteBuffer Pointer to data which is to be copied
 * @param DataTxSize Size of data to be sent
 * @return false if failed (message already prepared and not yet sent), true if successful
 */
bool ComModbus_1_writeMsg(uint16_t MsgIndx, uint8_t* ModWriteBuffer, uint8_t DataTxSize)
{
    bool success = 0;
    // get message pointer
    ComCfg_Modbus1MsgDataType* txMsgPtr = ComCfg_read_mb1Config((ComCfg_modbus0MsgIndxType) MsgIndx);

    // if message is not already waiting for TX then copy the data
    if (0 == txMsgPtr->mbRdyForTx)
    {
        // copy the data into the message pointer
        for (uint8_t i = 0; i < DataTxSize; i++)
        {
            txMsgPtr->dataOut[i] = ModWriteBuffer[i];
        }

        // add CRC
        unsigned int crc = CRC_16(txMsgPtr->dataOut, DataTxSize - 2);
        txMsgPtr->dataOut[DataTxSize - 2] = (crc >> 8) & 0xFF;  // CRC HI
        txMsgPtr->dataOut[DataTxSize - 1] = (crc >> 0) & 0xFF;  // CRC LOW

        // add send data size
        txMsgPtr->dataOutCount = DataTxSize;

        // flag message as ready for transmit
        txMsgPtr->mbRdyForTx = 1;

        // message was successfully copied
        success = 1;
    }
    return success;
}

void ComModbus_2_transceive(void* param)
{
    ComCfg_modbus2MsgIndxType sendingMsg = (ComCfg_modbus2MsgIndxType) 0;
    while (1)
    {
        if (COM_MODBUS_TRANSMITTING == _mb2TrnscieveType)
        {
            // loop through all of the messages to check if any is ready to be sent
            for (uint16_t mbMsgIndx = 0; mbMsgIndx < NUM_OF_MODBUS_2_MSG; mbMsgIndx++)
            {
                // get message pointer
                ComCfg_Modbus2MsgDataType* txMsgPtr = ComCfg_read_mb2Config((ComCfg_modbus2MsgIndxType) mbMsgIndx);

                // check if message is ready to be sent
                if (1 == txMsgPtr->mbRdyForTx)
                {
                    // Prepare for transmit
                    gpio_set_level(DE_RE_PIN_MB2, 1);

                    vTaskDelay(pdMS_TO_TICKS(1));

                    // write the data to UART line
                    uart_write_bytes(UART_MODBUS_2, (const char*) txMsgPtr->dataOut, txMsgPtr->dataOutCount);

                    vTaskDelay(pdMS_TO_TICKS(1));

                    // add 20ms for every 8 bytes of extra data to be sent
                    TickType_t timeoutTime = pdMS_TO_TICKS(100 + 20 * txMsgPtr->dataOutCount / 8);
                    uart_wait_tx_done(UART_MODBUS_2, timeoutTime);
                    // prepare for receive
                    gpio_set_level(DE_RE_PIN_MB2, 0);

                    // erase anything that is stuck in MODBUS 1 RX to prepare for response
                    uart_flush_input(UART_MODBUS_2);

                    // note which message index is ready to be sent
                    sendingMsg = (ComCfg_modbus2MsgIndxType) mbMsgIndx;
                    // break out of the for loop and continue with receive
                    _mb2TrnscieveType = COM_MODBUS_RECEIVING;
                    break;
                }
            }
        }
        if (COM_MODBUS_RECEIVING == _mb2TrnscieveType)
        {
            static uint8_t timeoutCntr = 0;  // counter of how many times the message has timed out

            // get message pointer
            ComCfg_Modbus2MsgDataType* rxMsgPtr = ComCfg_read_mb2Config(sendingMsg);

            // read the data
            uint8_t length = uart_read_bytes(UART_MODBUS_2, rxMsgPtr->dataIn, rxMsgPtr->dataInCount, pdMS_TO_TICKS(100));

            // if the data count is correct then proceed with parsing
            if (rxMsgPtr->dataInCount == length)
            {
                // clear the request for transmit flag as it was processed, set the request for parse flag. Reset the timeout counter.
                rxMsgPtr->mbRdyForTx = 0;
                rxMsgPtr->mbRdyForParse = 1;
                timeoutCntr = 0;

                // clear any leftover data which is erroneous
                uart_flush_input(UART_MODBUS_2);
            }
            else
            {
                timeoutCntr++;
                uart_flush_input(UART_MODBUS_2);

                if (timeoutCntr > MODBUS_TIMEOUT_COUNTER_MAX_VAL)
                {
                    timeoutCntr = 0;
                    // clear the transmit flag - the sending was not successful
                    rxMsgPtr->mbRdyForTx = 0;
                }
            }
            // return transcieving type to transmit
            _mb2TrnscieveType = COM_MODBUS_TRANSMITTING;
        }

        vTaskDelay(pdMS_TO_TICKS(COM_MODBUS2_TASK_DELAY_TIME_MS));
    }


    // delete task if illegal state was reached
    vTaskDelete( NULL );
}

/**
 * @brief Function is used to copy the data from ModWriteBuffer into the premade Modbus buffer and flagging the message for TX (For Modbus 2)
 * @param MsgIndx Message index for the array of Modbus messages
 * @param ModWriteBuffer Pointer to data which is to be copied
 * @param DataTxSize Size of data to be sent
 * @param DataRxSize Size of data to be received as response
 * @return false if failed (message already prepared and not yet sent), true if successful
 */
bool ComModbus_2_writeMsg(uint16_t MsgIndx, uint8_t* ModWriteBuffer, uint8_t DataTxSize, uint8_t DataRxSize)
{
    bool success = 0;
    // get message pointer
    ComCfg_Modbus2MsgDataType* txMsgPtr = ComCfg_read_mb2Config((ComCfg_modbus2MsgIndxType) MsgIndx);

    // if message is not already waiting for TX then copy the data
    if (0 == txMsgPtr->mbRdyForTx)
    {
        // copy the data into the message pointer
        for (uint8_t i = 0; i < DataTxSize; i++)
        {
            txMsgPtr->dataOut[i] = ModWriteBuffer[i];
        }

        // add CRC
        unsigned int crc = CRC_16(txMsgPtr->dataOut, DataTxSize - 2);
        txMsgPtr->dataOut[DataTxSize - 2] = (crc >> 8) & 0xFF;  // CRC HI
        txMsgPtr->dataOut[DataTxSize - 1] = (crc >> 0) & 0xFF;  // CRC LOW

        // add send and receive data size
        txMsgPtr->dataInCount = DataRxSize;
        txMsgPtr->dataOutCount = DataTxSize;

        // flag message as ready for transmit
        txMsgPtr->mbRdyForTx = 1;

        // message was successfully copied
        success = 1;
    }
    return success;
}