#include "ComCfg.h"

ComCfg_CanMsgDataType _canMsgs[NUM_OF_CAN_MSG];
ComCfg_Modbus1MsgDataType _modbus1Msgs[NUM_OF_MODBUS_1_MSG];
ComCfg_Modbus2MsgDataType _modbus2Msgs[NUM_OF_MODBUS_2_MSG];

void ComCfg_init(void)
{
    static bool init = false;

    if (false == init)
    {
        for (uint16_t i = 0; i < NUM_OF_CAN_MSG; i++)
        {
            _canMsgs[i].canRdyForTx = 0;                    // set transmit flag to 0 on init
            _canMsgs[i].canMsg.FIR.B.DLC = 8;               // all messages are DLC 8
            _canMsgs[i].canMsg.FIR.B.FF = CAN_frame_std;    // all messages are standard format
            _canMsgs[i].canMsg.data.u8[0] = 0;              // clear the data location
            _canMsgs[i].canMsg.data.u8[1] = 0;
            _canMsgs[i].canMsg.data.u8[2] = 0;
            _canMsgs[i].canMsg.data.u8[3] = 0;
            _canMsgs[i].canMsg.data.u8[4] = 0;
            _canMsgs[i].canMsg.data.u8[5] = 0;
            _canMsgs[i].canMsg.data.u8[6] = 0;
            _canMsgs[i].canMsg.data.u8[7] = 0;
        }

        for (uint16_t i = 0; i < NUM_OF_CAN_MSG; i++)
        {
            _modbus2Msgs[i].mbRdyForTx = 0;                  // set transmit flag to 0 on init
        }

        init = true;    // only init once
    }
}

ComCfg_CanMsgDataType* ComCfg_read_canConfig(uint16_t msgIndx)
{
    uint16_t rMsgIndx = 0;
    if (msgIndx < NUM_OF_CAN_MSG)
    {
        rMsgIndx = msgIndx;
    }
    return &_canMsgs[rMsgIndx];
}

/**
 * @brief Function returns pointer to Modbus 2 (UART2) configuration variable
 * @param msgIndx message index for the message config variable. See NUM_OF_MODBUS_2_MSG
 * @return returns pointer to modbus configuration variable ComCfg_Modbus2MsgDataType
 */
ComCfg_Modbus2MsgDataType* ComCfg_read_mb2Config(ComCfg_modbus2MsgIndxType msgIndx)
{
    uint16_t rMsgIndx = 0;
    if (msgIndx < NUM_OF_MODBUS_2_MSG)
    {
        rMsgIndx = msgIndx;
    }
    return &_modbus2Msgs[rMsgIndx];
}

/**
 * @brief Function returns pointer to Modbus 1 (UART0) configuration variable
 * @param msgIndx message index for the message config variable. See NUM_OF_MODBUS_1_MSG
 * @return returns pointer to modbus configuration variable ComCfg_Modbus1MsgDataType
 */
ComCfg_Modbus1MsgDataType* ComCfg_read_mb1Config(ComCfg_modbus0MsgIndxType msgIndx)
{
    uint16_t rMsgIndx = 0;
    if (msgIndx < NUM_OF_MODBUS_1_MSG)
    {
        rMsgIndx = msgIndx;
    }
    return &_modbus1Msgs[rMsgIndx];
}

uint16_t CRC_16(unsigned char* str, unsigned int usDataLen)
{
    uint8_t uchCRCHi = 0xFF;    // high byte of CRC initialized
    uint8_t uchCRCLo = 0xFF;    // low byte of CRC initialized
    uint16_t uIndex;    // will index into CRC lookup table
    while(usDataLen--)
    {
        uIndex = uchCRCHi ^ *str++; // calculate the CRC
        uchCRCHi = uchCRCLo ^ _auchCRCHi[uIndex];
        uchCRCLo = _auchCRCLo[uIndex];
    }
    return (uchCRCHi << 8 | uchCRCLo);
}

/**
 * @brief Function flags the CAN message for transmission so that it will be sent
 * @param MsgIndx message that should be sent
 * @return (void)
 */
void ComCfg_write_flagCanMsgForTx(ComCfg_canMsgIndxType MsgIndx)
{
    ComCfg_CanMsgDataType* msgPtr = ComCfg_read_canConfig(MsgIndx);
    msgPtr->canRdyForTx = 1;
}

/**
 * @brief Function clears the flag to CAN message parsing
 * @param MsgIndx message index for which the parsing flag should be cleared
 * @return (void)
 */
void ComCfg_clear_flagCanMsgForParse(ComCfg_canMsgIndxType MsgIndx)
{
    ComCfg_CanMsgDataType* msgPtr = ComCfg_read_canConfig(MsgIndx);
    msgPtr->canRdyForParse = 0;
}