#include "ComCan.h"

const twai_general_config_t g_genConfig =
{
    .mode = TWAI_MODE_NORMAL,
    .tx_io = GPIO_NUM_5,
    .rx_io = GPIO_NUM_4,
    .clkout_io = (gpio_num_t) -1,
    .bus_off_io = (gpio_num_t) -1,
    .tx_queue_len = 16,
    .rx_queue_len = 16,
    .alerts_enabled = 0,    // TODO(Nimfrodian) check documentation for alerts
    .clkout_divider = 0,
    .intr_flags = 0,    // TODO(Nimfrodian) check if this is needed
};

const twai_timing_config_t g_timingConfig = TWAI_TIMING_CONFIG_250KBITS();

const twai_filter_config_t g_filerConfig =
{
    .acceptance_code = 0,
    .acceptance_mask = 0,
    .single_filter = true,
};

void ComCan_init()
{
    // initialize CAN Module
    twai_driver_install(&g_genConfig, &g_timingConfig, &g_filerConfig);

    ComCfg_init();

    twai_start();

    // create receive task
    xTaskCreatePinnedToCore(
    ComCan_receive,    // Function that should be called
    "ComCan_receive",   // Name of the task (for debugging)
    8192,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL,            // Task handle
    1);              // run on core 1

    // create transmit task
    xTaskCreatePinnedToCore(
    ComCan_transmit,      // Function that should be called
    "ComCan_transmit",    // Name of the task (for debugging)
    8192 * 2,               // Stack size (bytes)
    NULL,                   // Parameter to pass
    1,                      // Task priority
    NULL,            // Task handle
    1);              // run on core 1
}

static void ComCan_saveMsg(ComCfg_canMsgIndxType msgIndx, twai_message_t* sourceMsgPtr)
{
    // get CAN message pointer to where the data should be saved
    ComCfg_CanMsgDataType* destMsgPtr = ComCfg_read_canConfig(msgIndx);

    // copy the data from source message to destination message
    for (uint8_t i = 0; i < sourceMsgPtr->data_length_code; i++)
    {
        destMsgPtr->canMsg.data[i] = sourceMsgPtr->data[i];
    }

    // save message ID
    destMsgPtr->canMsg.identifier = sourceMsgPtr->identifier;

    // flag that message is ready to be parsed
    destMsgPtr->canRdyForParse = 1;
}

/**
 * @brief Function reads and uses module parse on received message
 * @return (void)
 */
void ComCan_receive(void* param)
{
    while (1)
    {
        twai_message_t rxMessage;
        while (ESP_OK == twai_receive(&rxMessage, 0))  // TODO(Nimfrodian): Check if 0 for time is valid
        {
            uint32_t rxId = rxMessage.identifier;
            switch (rxId)
            {
                case 0x100:  // message CAN_MSG_RX_RELAY_CONTROL_WS_0
                {
                    ComCan_saveMsg(CAN_MSG_RX_RELAY_CONTROL_WS_0, &rxMessage);
                }
                break;
                case 0x105:  // message CAN_MSG_RX_RELAY_DISABLE_WS_0
                {
                    ComCan_saveMsg(CAN_MSG_RX_RELAY_DISABLE_WS_0, &rxMessage);
                }
                break;
                break;

                default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(COM_CAN_TASK_DELAY_TIME_MS));
    }

    // delete task if illegal state was reached
    vTaskDelete(NULL);
}

void ComCan_transmit(void* param)
{
    while (1)
    {
        // check if any messages are waiting to be sent
        for (uint16_t msgIndx = 0; msgIndx < NUM_OF_CAN_MSG; msgIndx++)
        {
            // get message data pointer
            ComCfg_CanMsgDataType* canMsgPtr = ComCfg_read_canConfig(msgIndx);

            // check if message needs to be sent
            if (1 == canMsgPtr->canRdyForTx)
            {
                // try to copy the message into hardware transmit buffer
                if (ESP_OK == twai_transmit(&canMsgPtr->canMsg, 10))
                {
                    // clear "send" flag if copy was successful
                    canMsgPtr->canRdyForTx = 0;
                    break;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(COM_CAN_TASK_DELAY_TIME_MS));
    }

    // delete task if illegal state was reached
    vTaskDelete(NULL);
}