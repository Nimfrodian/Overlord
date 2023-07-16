#include "RelayControl.h"

static uint8_t _currRlyStsMb0[RELAY_MODULE_NUM_OF_RELAY_BOARDS] = {0};                   // current relay status as a bitmap for Waveshare relay board
static uint8_t _canReqWaveshareRelayInvert[8];                                  // request for inversion of Waveshare relay statuses. Limited to 8 (64) relays as that is the number of bits a CAN message can have

/**
 * @brief Function initialises the relay module variables.
 * @param void
 * @return (void)
 */
void RelayControl_init(void)
{
    // set all current relay statuses to 0
    for (uint8_t modIndx = 0; modIndx < RELAY_MODULE_NUM_OF_RELAY_BOARDS; modIndx++)
    {
        _currRlyStsMb0[modIndx] = 0x00;
    }
}

/**
 * @brief Function composes Modbus message for WAVESHARE relay board.
 * @param ModuleIndx Module index for which the message is to be prepared
 * @param MbMsgData Modbus data pointer. Data is of size RELAY_MODULE_MB0_TRANSMIT_MSG_SIZE
 * @param GpioSt Current GPIO status bitmap of the corresponding GPIO input. On GPIO state change the relay is also changed
 * @return true if message was state change occured, false if not
 */
bool RelayControl_composeWaveshareModbusMessage(uint8_t ModuleIndx, uint8_t* MbMsgData, uint8_t GpioSt)
{
    // create return variable
    bool stateChange = false;

    uint8_t invrtSt = 0x00;
    for (uint8_t relIndx = 0; relIndx < NUM_OF_RELAYS_PER_WAVESHARE_BOARD; relIndx++)
    {
        invrtSt |= (_canReqWaveshareRelayInvert[ModuleIndx] & (0x01 << relIndx));  // invert status as received from CAN
    }

    // calculate requested relay state
    uint8_t newRlySt = (GpioSt ^ invrtSt);

    // if new relay state is different then what is on board then change it
    if (_currRlyStsMb0[ModuleIndx] != newRlySt)
    {
        stateChange = true;
    }

    // Modbus message structure for relay board
    {
        MbMsgData[0] = ((char) ModuleIndx) + 0x01;    // ID
        MbMsgData[1] = 0x0F;        // function code
        MbMsgData[2] = 0x00;        // Relay start address
        MbMsgData[3] = 0x00;        // Relay start address
        MbMsgData[4] = 0x00;        // Number of relays
        MbMsgData[5] = 0x08;        // Number of relays
        MbMsgData[6] = 0x01;        // following bytes
        MbMsgData[7] = newRlySt;    // bitmap of new relay statuses
        MbMsgData[8] = 0x00;        // placeholder for CRC
        MbMsgData[9] = 0x00;        // placeholder for CRC

        _currRlyStsMb0[ModuleIndx] = newRlySt;
    }

    return stateChange;
}

/**
 * @brief Function parses CAN message in relation to relay inversion
 * @param CanData Pointer to CAN data
 * @param CanId CAN ID of the message
 * @return (void)
 */
void RelayControl_parseCanMessage(uint8_t* CanData, uint32_t CanId)
{
    if (0x100 == CanId)
    {
        _canReqWaveshareRelayInvert[0] = CanData[0];
        _canReqWaveshareRelayInvert[1] = CanData[1];
        _canReqWaveshareRelayInvert[2] = CanData[2];
        _canReqWaveshareRelayInvert[3] = CanData[3];
        _canReqWaveshareRelayInvert[4] = CanData[4];
        _canReqWaveshareRelayInvert[5] = CanData[5];
        _canReqWaveshareRelayInvert[6] = CanData[6];
        _canReqWaveshareRelayInvert[7] = CanData[7];
    }
}

/**
 * @brief Function composes CAN message with updated relay statuses
 * @param MsgData Pointer to CAN message data
 * @param CanId CAN message ID
 * @return (void)
 */
void RelayControl_composeCanMessage(uint8_t* MsgData, uint32_t* CanId)
{
        // prepare message with relay statuses
        for (uint8_t i = 0; i < 8; i++)
        {
            // clear content
            MsgData[i] = 0x00;
        }
        for (uint8_t modIndx = 0; modIndx < RELAY_MODULE_NUM_OF_RELAY_BOARDS; modIndx++)
        {
            MsgData[modIndx] = _currRlyStsMb0[modIndx];
        }

        // set CAN ID
        *CanId = (uint32_t) 0x110;
}