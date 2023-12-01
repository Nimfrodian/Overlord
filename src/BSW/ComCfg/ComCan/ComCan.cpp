#include "ComCan.h"

CAN_device_t CAN_cfg;

void ComCan_init()
{
    CAN_cfg.speed = CAN_SPEED_250KBPS;
    CAN_cfg.tx_pin_id = GPIO_NUM_5;
    CAN_cfg.rx_pin_id = GPIO_NUM_4;
    /* create a queue for CAN receiving */
    CAN_cfg.rx_queue = xQueueCreate(10,sizeof(CAN_frame_t));
    //initialize CAN Module
    ESP32Can.CANInit();

    ComCfg_init();

    // create receive task
    xTaskCreatePinnedToCore(
    ComCan_receive,    // Function that should be called
    "ComCan_receive",   // Name of the task (for debugging)
    8192,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL,            // Task handle
    1                // run on core 1
    );

    // create transmit task
    xTaskCreatePinnedToCore(
    ComCan_transmit,      // Function that should be called
    "ComCan_transmit",    // Name of the task (for debugging)
    8192 * 2,               // Stack size (bytes)
    NULL,                   // Parameter to pass
    1,                      // Task priority
    NULL,            // Task handle
    1                // run on core 1
    );
}

static void ComCan_saveMsg(ComCfg_canMsgIndxType msgIndx, CAN_frame_t* sourceMsgPtr)
{
    // get CAN message pointer to where the data should be saved
    ComCfg_CanMsgDataType* destMsgPtr = ComCfg_get_canConfig(msgIndx);

    // copy the data from source message to destination message
    for (uint8_t i = 0; i < 8; i++)
    {
        destMsgPtr->canMsg.data.u8[i] = sourceMsgPtr->data.u8[i];
    }

    // save message ID
    destMsgPtr->canMsg.MsgID = sourceMsgPtr->MsgID;

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
        CAN_frame_t _rxFrame;
        if (pdTRUE == xQueueReceive(CAN_cfg.rx_queue, &_rxFrame, 3*portTICK_PERIOD_MS))
        {
            uint32_t rxId = _rxFrame.MsgID;
            switch (rxId)
            {
                case 0x100: // message CAN_MSG_RX_RELAY_CONTROL_WS_0
                {
                    ComCan_saveMsg(CAN_MSG_RX_RELAY_CONTROL_WS_0, &_rxFrame);
                }
                break;
                case 0x105: // message CAN_MSG_RX_RELAY_DISABLE_WS_0
                {
                    ComCan_saveMsg(CAN_MSG_RX_RELAY_DISABLE_WS_0, &_rxFrame);
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
    vTaskDelete( NULL );
}

void ComCan_transmit(void* param)
{
    while (1)
    {
        // check if any messages are waiting to be sent
        for (uint16_t msgIndx = 0; msgIndx < NUM_OF_CAN_MSG; msgIndx++)
        {
            // get message data pointer
            ComCfg_CanMsgDataType* canMsgPtr = ComCfg_get_canConfig(msgIndx);

            // check if message needs to be sent
            if (1 == canMsgPtr->canRdyForTx)
            {
                // try to copy the message into hardware transmit buffer
                if (0 == ESP32Can.CANWriteFrame(&canMsgPtr->canMsg))
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
    vTaskDelete( NULL );
}