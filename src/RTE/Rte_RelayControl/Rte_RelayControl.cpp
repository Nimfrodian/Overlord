#include "RTE.h"
#include "RelayControl.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "ComCan.h"
#include "ComModbus.h"

const uint16_t RELAY_STS_UPDATE_INTERVAL_MS = 1000;

static bool _rlyStChange = 0;   // relay state change flag. CAN message is sent if any relays changed state

void Rte_RelayControl_init(void)
{
    RelayControl_init();
}

void Rte_RelayControl_runnable_10ms(void)
{
    // send Modbus message command for Waveshare relay module
    {
        for (uint8_t modIndx = 0; modIndx < RELAY_MODULE_NUM_OF_RELAY_BOARDS; modIndx++)
        {
            // get Modbus message pointer
            ComCfg_Modbus0MsgDataType* txMsgPtr = ComCfg_get_mb0Config((ComCfg_modbus0MsgIndxType) (MODBUS_0_MSG_SET_ALL_RELAYS_0_7 + modIndx));

            // proceed to sending Modbus message if it is not already being sent or parsed
            if ((0 == txMsgPtr->mbRdyForTx) && (0 == txMsgPtr->mbRdyForParse))
            {
                // create bitfield variable for relay states
                uint32_t gpioStBitfield = 0x00;

                // check all corresponding GPIO statuses for relays
                for (uint8_t relIndx = 0; relIndx < NUM_OF_RELAYS_PER_WAVESHARE_BOARD; relIndx++)
                {
                    if ((relIndx < NUM_OF_INPUT_INDX))
                    {
                        gpioStBitfield = gpioStBitfield | (Rte_Dio_get_gpioSt(relIndx) << relIndx);
                    }
                }

                // create temporary variable for data and compose the Modbus message
                uint8_t mbData[RELAY_MODULE_MB0_TRANSMIT_MSG_SIZE] = {0};
                _rlyStChange |= RelayControl_composeWaveshareModbusMessage(modIndx, mbData, gpioStBitfield);

                // since feedback is not monitored for Waveshare board the message should be sent constantly and as fast as possible
                {
                    // copy the data to internal Modbus buffer
                    (void) ComModbus_0_writeMsg((MODBUS_0_MSG_SET_ALL_RELAYS_0_7 + modIndx), mbData, RELAY_MODULE_MB0_TRANSMIT_MSG_SIZE);
                }
            }
        }
    }

    // parse CAN messages
    {
        for (uint32_t canMsgIndx = CAN_MSG_RX_RELAY_CONTROL_WS_0; canMsgIndx <= CAN_MSG_RX_RELAY_CONTROL_WS_0; canMsgIndx++)
        {
            // get message pointer
            ComCfg_CanMsgDataType* msgPtr = ComCfg_get_canConfig(canMsgIndx);

            if (true == msgPtr->canRdyForParse)
            {
                // parse CAN message
                uint8_t* canData = &msgPtr->canMsg.data.u8[0];
                uint32_t canMsgId = msgPtr->canMsg.MsgID;
                RelayControl_parseCanMessage(canData, canMsgId);

                // CAN message was parsed, clear the parsing flag
                ComCfg_clear_flagCanMsgForParse((ComCfg_canMsgIndxType) canMsgIndx);
            }
        }
    }

    // send CAN message on relay states
    {
        static uint16_t cntr = 0;
        cntr += 10;

        // if a change was made on any board the CAN message should be prepared. The message should also be sent periodically
        if ((1 == _rlyStChange) || (RELAY_STS_UPDATE_INTERVAL_MS <= cntr))
        {
            {
                // get message pointer
                ComCfg_CanMsgDataType* msgPtr = ComCfg_get_canConfig(CAN_MSG_TX_RELAY_STATUS);

                // prepare CAN data
                uint8_t* canDataPtr = &msgPtr->canMsg.data.u8[0];
                uint32_t* canIdPtr = &msgPtr->canMsg.MsgID;
                RelayControl_composeCanMessage(canDataPtr, canIdPtr);

                // flag for transmission
                ComCfg_set_flagCanMsgForTx(CAN_MSG_TX_RELAY_STATUS);
            }

            // clear timing counter
            cntr = 0;

            // relay state change was acknowledged so clear flag
            _rlyStChange = 0;
        }
    }
}