#include "SDM120M.h"

SDM120M_dataType _rxData[SDM120M_NUM_OF_MODULES][SDM120M_NUM_OF_READ]; // parsed values

/**
 * @brief Function initialises internal data
 * @param void
 * @return (void)
 */
void Sdm120m_init(void)
{
    for (uint8_t modIndx = 0; modIndx < SDM120M_NUM_OF_MODULES; modIndx++)
    {
        for (uint8_t msgIndx = 0; msgIndx < SDM120M_NUM_OF_READ; msgIndx++)
        {
            _rxData[modIndx][msgIndx].dataReadyFlg = 0;
        }
    }
}

/**
 * @brief Function parses Modbus data
 * @param ModuleIndx SDM120M module index
 * @param VariableIndx Variable index as per SDM120M_NUM_OF_READ
 * @param DataIn pointer to data from SDM120M that holds the data
 * @return (void)
 */
void Sdm120m_parseModbusData(uint8_t ModuleIndx, uint8_t VariableIndx, uint8_t* DataIn)
{
    // parse the data
    {
        // combine bytes into uint32_t value
        uint32_t temp = ((uint32_t)DataIn[3] << 24) |
                        ((uint32_t)DataIn[4] << 16) |
                        ((uint32_t)DataIn[5] << 8)  |
                        ((uint32_t)DataIn[6] << 0);
        float result = *((float *)&temp); // convert uint32_t value to float variable

        // store the result
        _rxData[ModuleIndx][VariableIndx].val = result;

        // message was parsed set parsed flag
        _rxData[ModuleIndx][VariableIndx].dataReadyFlg = 1;
    }
}

/**
 * @brief Function composes Modbus data query command
 * @param ModMsgData Pointer to Modbus data
 * @param MsgIndx Message index. Each command has different data registers so this variable chooses which one to send
 * @param ModuleIndx Module index for which the message should be prepared
 * @return (void)
 */
void Sdm120m_modbus_compose(uint8_t* ModMsgData, uint8_t MsgIndx, uint8_t ModuleIndx)
{
    // fill modbus message data
    ModMsgData[0] = _sdm120m_mb_module_id[ModuleIndx];      // ID
    ModMsgData[1] = 0x04;                                   // Function code
    ModMsgData[2] = (_sdm120m_mb_start_addrs[MsgIndx] >> 8) & 0xFF;   // Data Reg High HI
    ModMsgData[3] = (_sdm120m_mb_start_addrs[MsgIndx] >> 0) & 0xFF;   // Data Reg High LO
    ModMsgData[4] = 0x00;                                   // Data Reg Low  HI
    ModMsgData[5] = 0x02;                                   // Data Reg Low  LO
    ModMsgData[6] = 0x00;                                   // CRC
    ModMsgData[7] = 0x00;                                   // CRC
}


/**
 * @brief Function composes CAN data based on input parameters
 * @param CanData Pointer to CAN data
 * @param CanId Pointer to variable that holds the CAN ID
 * @param Value1 Value 1 to be sent
 * @param Value2 Value 2 to be sent. Value 1 and 2 are consecutive values as per SDM120M_NUM_OF_READ
 * @param CanMsgIndx CAN message index (each SDM120M module has several CAN messages)
 * @param ModuleIndx SDM120M module index
 * @return (void)
 */
void Sdm120m_can_compose(uint8_t* CanData, uint32_t* CanId, uint32_t Value1, uint32_t Value2, uint32_t CanMsgIndx, uint8_t ModuleIndx)
{
    // prepare message data
    CanData[0] = (Value1 >> 0)  & 0xFF;
    CanData[1] = (Value1 >> 8)  & 0xFF;
    CanData[2] = (Value1 >> 16) & 0xFF;
    CanData[3] = (Value1 >> 24) & 0xFF;
    CanData[4] = (Value2 >> 0)  & 0xFF;
    CanData[5] = (Value2 >> 8)  & 0xFF;
    CanData[6] = (Value2 >> 16) & 0xFF;
    CanData[7] = (Value2 >> 24) & 0xFF;

    // prepare message ID
    *CanId = 0x1F5 + CanMsgIndx + (ModuleIndx * SDM120M_CAN_TX_MSG_TYPE_NUM);

}

/**
 * @brief Function returns data ready flag. If data was received from module this will be set
 * @param ModuleIndx SDM120M module index
 * @param VarIndx Variable index as per SDM120M_NUM_OF_READ. Each variable has a data ready flag.
 * @return true if data is ready
 */
bool Sdm120m_get_dataReadyFlag(uint8_t ModuleIndx, uint8_t VarIndx)
{
    return _rxData[ModuleIndx][VarIndx].dataReadyFlg;
}

/**
 * @brief Function returns the data value
 * @param ModuleIndx SDM120M module index
 * @param VarIndx Variable index as per SDM120M_NUM_OF_READ.
 * @return
 */
float Sdm120m_get_dataValue(uint8_t ModuleIndx, uint8_t VarIndx)
{
    return _rxData[ModuleIndx][VarIndx].val;
}

/**
 * @brief Function clears the data ready flag so that new data will be read
 * @param ModuleIndx SDM120M module index
 * @param MsgIndx1 Message index 1.
 * @param MsgIndx2 Message index 2. Each query returns two variables hence MsgIndx1 and MsgIndx2. Both need to be cleared for new query to be sent
 * @return (void)
 */
void Sdm120m_clear_dataReadyFlag(uint8_t ModuleIndx, uint8_t MsgIndx1, uint8_t MsgIndx2)
{
    // clear origin data ready flag
    _rxData[ModuleIndx][MsgIndx1].dataReadyFlg = 0;
    if (SDM120M_CAN_TX_MSG_TYPE_NUM != (MsgIndx2))
    {
        _rxData[ModuleIndx][MsgIndx2].dataReadyFlg = 0;
    }
}