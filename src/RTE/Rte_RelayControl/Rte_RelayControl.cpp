#include "RTE.h"
#include "RelayControl.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "ComCan.h"
#include "ComModbus.h"

const uint16_t RELAY_STS_UPDATE_INTERVAL_MS = 1000;
const uint16_t RELAY_MB_MSG_UPDATE_INTERVAL_MS = 100;

static bool _rlyStChange = 0;   // relay state change flag. CAN message is sent if any relays changed state
static uint32_t _rlyStChangeTi = 0;

void Rte_RelayControl_init(void)
{
    RelayControl_init();

    // clear all flags for messages that will be used
    for (uint16_t mbMsgIndx = MODBUS_1_MSG_SET_ALL_RELAYS_0_7; mbMsgIndx < NUM_OF_MODBUS_1_MSG; mbMsgIndx++)
    {
        ComCfg_Modbus1MsgDataType* mbMsgPtr = ComCfg_read_mb1Config((ComCfg_modbus0MsgIndxType) mbMsgIndx);
        mbMsgPtr->mbRdyForTx = 0;
        mbMsgPtr->mbRdyForParse = 1;
    }
}

void Rte_RelayControl_runnable_10ms(void)
{
    // send Modbus message command for Waveshare relay module
    {
        for (uint8_t modIndx = 0; modIndx < RELAY_MODULE_NUM_OF_RELAY_BOARDS; modIndx++)
        {
            // get Modbus message pointer
            ComCfg_Modbus1MsgDataType* txMsgPtr = ComCfg_read_mb1Config((ComCfg_modbus0MsgIndxType) (MODBUS_1_MSG_SET_ALL_RELAYS_0_7 + modIndx));

            static uint8_t rlyToUpdate = 0;

            // proceed to sending Modbus message if it is not already being sent
            if ((0 == txMsgPtr->mbRdyForTx) && (1 == txMsgPtr->mbRdyForParse))
            {

                // create bitfield variable for relay states
                uint32_t gpioStBitfield = 0x00;

                // check all corresponding GPIO statuses for relays
                for (uint8_t currRelIndx = 0; currRelIndx < NUM_OF_RELAYS_PER_WAVESHARE_BOARD; currRelIndx++)
                {
                    uint8_t actRelIndx = currRelIndx + modIndx * NUM_OF_RELAYS_PER_WAVESHARE_BOARD; // actual relay index from 0 - 31 (at 4 boards of 8 relays)
                    if ((actRelIndx < NUM_OF_INPUT_INDX))
                    {
                        uint8_t actRelIndx = currRelIndx + modIndx * NUM_OF_RELAYS_PER_WAVESHARE_BOARD;
                        gpioStBitfield = gpioStBitfield | (Rte_Dio_read_gpioSt(actRelIndx) << currRelIndx);
                    }
                }

                // create temporary variable for data and compose the Modbus message
                uint8_t mbData[RELAY_MODULE_MB0_TRANSMIT_MSG_SIZE] = {0};
                bool rlyStChangeLocal = RelayControl_composeWaveshareModbusMessage(modIndx, mbData, gpioStBitfield);
                _rlyStChange |= rlyStChangeLocal;

                bool rlyTiUpdate = (RELAY_MB_MSG_UPDATE_INTERVAL_MS <= _rlyStChangeTi) && (modIndx == rlyToUpdate);
                bool rlyUpdate = rlyStChangeLocal || rlyTiUpdate;

                // since feedback is not monitored for Waveshare board the message should be sent constantly and as fast as possible
                if (rlyUpdate)
                {
                    _rlyStChangeTi = 0;
                    txMsgPtr->mbRdyForParse = 0;
                    // copy the data to internal Modbus buffer
                    uint16_t modMsgIndx = (MODBUS_1_MSG_SET_ALL_RELAYS_0_7 + modIndx);
                    (void) ComModbus_1_writeMsg(modMsgIndx, mbData, RELAY_MODULE_MB0_TRANSMIT_MSG_SIZE);

                    // reset timer
                    rlyToUpdate++;
                    if (rlyToUpdate >= RELAY_MODULE_NUM_OF_RELAY_BOARDS)
                    {
                        rlyToUpdate = 0;
                    }
                    _rlyStChangeTi = 0;
                }
            }
        }
        _rlyStChangeTi += 10;
    }

    // parse CAN messages
    {
        for (uint32_t canMsgIndx = CAN_MSG_RX_RELAY_CONTROL_WS_0; canMsgIndx <= CAN_MSG_RX_RELAY_CONTROL_WS_0; canMsgIndx++)
        {
            // get message pointer
            ComCfg_CanMsgDataType* msgPtr = ComCfg_read_canConfig(canMsgIndx);

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
        for (uint32_t canMsgIndx = CAN_MSG_RX_RELAY_DISABLE_WS_0; canMsgIndx <= CAN_MSG_RX_RELAY_DISABLE_WS_0; canMsgIndx++)
        {
            // get message pointer
            ComCfg_CanMsgDataType* msgPtr = ComCfg_read_canConfig(canMsgIndx);

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
                ComCfg_CanMsgDataType* msgPtr = ComCfg_read_canConfig(CAN_MSG_TX_RELAY_STATUS);

                // prepare CAN data
                uint8_t* canDataPtr = &msgPtr->canMsg.data.u8[0];
                uint32_t* canIdPtr = &msgPtr->canMsg.MsgID;
                RelayControl_composeCanMessage(canDataPtr, canIdPtr);

                // flag for transmission
                ComCfg_write_flagCanMsgForTx(CAN_MSG_TX_RELAY_STATUS);
            }

            // clear timing counter
            cntr = 0;

            // relay state change was acknowledged so clear flag
            _rlyStChange = 0;
        }
    }
}

bool Rte_Relay_read_relaySt(uint8_t RelayIndx)
{
    bool state = false;
    if (RelayIndx < (RELAY_MODULE_NUM_OF_RELAY_BOARDS * 8))
    {
        state = RelayControl_read_relaySt(RelayIndx);
    }
    return state;
}

bool Rte_Relay_write_relaySt(uint8_t RelayIndx, bool State)
{
    bool relaySet = false;
    if (RelayIndx < (RELAY_MODULE_NUM_OF_RELAY_BOARDS * 8))
    {
        relaySet = RelayControl_write_relaySt(RelayIndx, State);
    }
    return relaySet;
}