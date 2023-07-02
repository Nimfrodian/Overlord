#include "RTE.h"
#include "SDM120M.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "ComCan.h"
#include "ComModbus.h"

static uint32_t _sdm120m_msgSendIntrvlCntr_ms[SDM120M_NUM_OF_MODULES][SDM120M_NUM_OF_READ] = {0};  // interval counter. Different Modbus message should be sent with different intervals
static const uint32_t _sdm120m_msgSendIntrvSetpoint_ms[SDM120M_NUM_OF_READ] =  // interaval setpoint. Each message is only sent once the counter for it reaches this predefined value. NOTE: since CAN message is sent once the pair of values is available each pair should have the same counter value
{
    [SDM120M_READ_VOLTAGE_V] = 1000,
    [SDM120M_READ_CURRENT_A] = 1000,
    [SDM120M_ACTIVE_POWER_W] = 1000,
    [SDM120M_APPARENT_POWER_VA] = 1000,
    [SDM120M_REACTIVE_POWER_VAr] = 5000,
    [SDM120M_POWER_FACTOR] = 5000,
    [SDM120M_FREQUENCY] = 5000,
    [SDM120M_IMPORT_ACTIVE_ENERGY_kWh] = 5000,
    [SDM120M_EXPORT_ACTIVE_ENERGY_kWh] = 60000,
    [SDM120M_IMPORT_REACTIVE_ENERGY_kVArh] = 60000,
    [SDM120M_EXPORT_REACTIVE_ENERGY_kVArh] = 60000,
    [SDM120M_TOTAL_SYSTEM_POWER_DEMAND_W] = 60000,
    [SDM120M_MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND_W] = 5000,
    [SDM120M_IMPORT_SYSTEM_POWER_DEMAND_W] = 5000,
    [SDM120M_MAXIMUM_IMPORT_SYSTEM_POWER_DEMAND_W] = 60000,
    [SDM120M_EXPORT_SYSTEM_POWER_DEMAND_W] = 60000,
    [SDM120M_MAXIMUM_EXPORT_SYSTEM_POWER_DEMAND] = 60000,
    [SDM120M_CURRENT_DEMAND_A] = 60000,
    [SDM120M_MAXIMUM_CURRENT_DEMAND_A] = 5000,
    [SDM120M_TOTAL_ACTIVE_ENERGY_kWh] = 5000,
    [SDM120M_TOTAL_REACTIVE_ENERGY_kVArh] = 60000,
};

void Rte_Sdm120m_init(void)
{
    Sdm120m_init();

    // clear all flags for messages that will be used
    const uint16_t lastMbMsgIndxInit = MODBUS_2_MSG_SDM120M_01_READ_VOLTAGE_V + (SDM120M_NUM_OF_READ * SDM120M_NUM_OF_MODULES) - 1;
    for (uint16_t mbMsgIndx = MODBUS_2_MSG_SDM120M_01_READ_VOLTAGE_V; mbMsgIndx <= lastMbMsgIndxInit; mbMsgIndx++)
    {
        ComCfg_Modbus2MsgDataType* mbMsgPtr = ComCfg_get_mb2Config((ComCfg_modbus2MsgIndxType) mbMsgIndx);
        mbMsgPtr->mbRdyForTx = 0;
        mbMsgPtr->mbRdyForParse = 0;
    }
}

void Rte_Sdm120m_runnable_10ms(void)
{
    // send Modbus messages
    {
        // prepare all modbus messages to be sent
        for (uint8_t modIndx = 0; modIndx < SDM120M_NUM_OF_MODULES; modIndx++)
        {
            for (uint8_t msgIndx = 0; msgIndx < SDM120M_NUM_OF_READ; msgIndx++)
            {
                // calculate modbus message index
                uint16_t mbMsgIndx = MODBUS_2_MSG_SDM120M_01_READ_VOLTAGE_V + (modIndx * SDM120M_NUM_OF_READ) + (msgIndx);

                // get message pointer
                ComCfg_Modbus2MsgDataType* msgPtr = ComCfg_get_mb2Config((ComCfg_modbus2MsgIndxType) mbMsgIndx);

                // if message is not already being serviced then check if it needs sending or tracking of time
                if (false == msgPtr->mbRdyForTx)
                {
                    // if enough time has passed then send the message
                    if (_sdm120m_msgSendIntrvlCntr_ms[modIndx][msgIndx] >= _sdm120m_msgSendIntrvSetpoint_ms[msgIndx])
                    {
                        // create temporary modbus data holder
                        uint8_t modMsgData[SDM120M_MODBUS_TX_DATA_SIZE] = {0};

                        // fill the data holder with data
                        Sdm120m_modbus_compose(&modMsgData[0], msgIndx, modIndx);

                        // write the message
                        (void) ComModbus_2_writeMsg(mbMsgIndx, modMsgData, SDM120M_MODBUS_TX_DATA_SIZE, SDM120M_MODBUS_RX_DATA_SIZE);

                        // message was parsed, clear inbound flag
                        msgPtr->mbRdyForParse = 0;

                        // reset counter when message is read
                        _sdm120m_msgSendIntrvlCntr_ms[modIndx][msgIndx] = 0;
                    }
                    else  // else increment counter to measure time
                    {
                        // increment send counter. This is used to delay messages to not clog the Modbus line
                        _sdm120m_msgSendIntrvlCntr_ms[modIndx][msgIndx]+=10;
                    }
                }
            }
        }
    }

    // parse any available data
    {
        for (uint16_t mbMsgIndx = MODBUS_2_MSG_SDM120M_01_READ_VOLTAGE_V; mbMsgIndx <= MODBUS_2_MSG_SDM120M_20_TOTAL_REACTIVE_ENERGY_kVArh; mbMsgIndx++)
        {
            // get message pointer
            ComCfg_Modbus2MsgDataType* msgPtr = ComCfg_get_mb2Config((ComCfg_modbus2MsgIndxType) mbMsgIndx);

            // determine if message needs parsing
            if (1 == msgPtr->mbRdyForParse)
            {
                // calculate variable index
                uint16_t varIndx = (mbMsgIndx - MODBUS_2_MSG_SDM120M_01_READ_VOLTAGE_V) % SDM120M_NUM_OF_READ;

                // calculate SDM120M module index
                uint16_t modIndx = (mbMsgIndx - MODBUS_2_MSG_SDM120M_01_READ_VOLTAGE_V) / SDM120M_NUM_OF_READ;

                // parse the incoming data
                Sdm120m_parseModbusData(modIndx, varIndx, &msgPtr->dataIn[0]);

                // message was parsed, clear inbound flag
                msgPtr->mbRdyForParse = 0;
            }
        }
    }

    // send CAN data
    {
        // prepare all module messages to be sent
        for (uint8_t modIndx = 0; modIndx < SDM120M_NUM_OF_MODULES; modIndx++)
        {
            for (uint8_t canMsgIndx = 0; canMsgIndx < SDM120M_CAN_TX_MSG_TYPE_NUM; canMsgIndx++)
            {
                // determine indexing
                uint8_t msg1Indx = canMsgIndx * 2;
                uint8_t msg2Indx = canMsgIndx * 2 + 1;

                // check if both values of the message are ready to be sent
                bool valRdy1 = 1;   // value 1 ready to be sent flag
                bool valRdy2 = 1;   // value 2 ready to be sent flag
                uint32_t val1 = 0;  // actual value 1 to be sent (raw data)
                uint32_t val2 = 0;  // actual value 2 to be sent (raw data)

                // variable 1 acquisition
                {
                    // get data ready flag
                    valRdy1 = Sdm120m_get_dataReadyFlag(modIndx, msg1Indx);

                    // get the actual value
                    float tempValue = Sdm120m_get_dataValue(modIndx, msg1Indx);
                    val1 = *((uint32_t*) ((void*) &tempValue));
                }

                // variable 2 acquisition
                {
                    // if second variable is valid, i.e. not end of data array
                    if (SDM120M_NUM_OF_READ != (msg2Indx))
                    {
                        // get data ready flag
                        valRdy2 = Sdm120m_get_dataReadyFlag(modIndx, msg2Indx);

                        // get the actual value
                        float tempValue = Sdm120m_get_dataValue(modIndx, msg2Indx);
                        val2 = *((uint32_t*) ((void*) &tempValue));
                    }
                }

                // if both values for the message are ready to be sent then prepare CAN message
                if (valRdy1 && valRdy2)
                {
                    // determine CAN message pointer index
                    uint16_t canMsgGlblIndx = CAN_MSG_TX_SDM120M_01_1 + canMsgIndx + (modIndx * SDM120M_CAN_TX_MSG_TYPE_NUM);   // global CAN message index as defined in ComCfg.h

                    // get CAN message pointer
                    ComCfg_CanMsgDataType* msgPtr = ComCfg_get_canConfig(canMsgGlblIndx);

                    // compose CAN data
                    Sdm120m_can_compose(&msgPtr->canMsg.data.u8[0], &msgPtr->canMsg.MsgID, val1, val2, canMsgIndx, modIndx);

                    // data was prepared, clear it for new data
                    Sdm120m_clear_dataReadyFlag(modIndx, msg1Indx, msg2Indx);

                    // flag for transmit
                    ComCfg_set_flagCanMsgForTx((ComCfg_canMsgIndxType) canMsgGlblIndx);
                }
            }
        }
    }
}