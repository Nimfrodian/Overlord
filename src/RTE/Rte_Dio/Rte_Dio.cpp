#include "Dio_Cfg.h"
#include "Dio.h"
#include "ComCfg.h"

void Rte_Dio_init(void)
{

}

/**
 * @brief Function returns the state of the GPIO pin if GpioIndx is in the correct range
 * @param GpioIndx GPIO index for which to get value
 * @return GPIO state
 */
bool Rte_Dio_read_gpioSt(uint8_t GpioIndx)
{
    bool gpioSt = false;
    gpioSt = BSW_Dio_read_inputGpioSt((BSW_Dio_inputIndxType) GpioIndx);
    return gpioSt;
}

void Rte_Dio_runnable_10ms(void)
{
    // send CAN message with GPIO state
    {
        static uint32_t cntr_ms = 0;
        cntr_ms += 10;

        if (100 == cntr_ms)
        {
            // determine CAN message pointer index
            uint16_t canMsgIndx = CAN_MSG_TX_DI_STATUS;   // CAN message index as defined in ComCfg.h

            // get CAN message pointer
            ComCfg_CanMsgDataType* msgPtr = ComCfg_read_canConfig(canMsgIndx);

            // create array of GPIO input values
            bool gpioInArr[NUM_OF_INPUT_INDX] = {0};
            for (uint8_t gpioInIndx = 0; gpioInIndx < NUM_OF_INPUT_INDX; gpioInIndx++)
            {
                gpioInArr[gpioInIndx] = Rte_Dio_read_gpioSt(gpioInIndx);
            }

            // compose CAN data
            Dio_can_compose(&msgPtr->canMsg.data.u8[0], &msgPtr->canMsg.MsgID, &gpioInArr[0]);

            // flag for transmit
            ComCfg_write_flagCanMsgForTx((ComCfg_canMsgIndxType) canMsgIndx);

            cntr_ms = 0;
        }
    }
}