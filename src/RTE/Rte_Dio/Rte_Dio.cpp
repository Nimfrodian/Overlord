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
bool Rte_Dio_get_gpioSt(uint8_t GpioIndx)
{
    bool gpioSt = false;
    gpioSt = BSW_Dio_read_inputGpioSt((BSW_Dio_inputIndxType) GpioIndx);
    return gpioSt;
}

/**
 * @brief Function sets the GPIO debug pin to desired state for debugging purposes
 * @param GpioSts gpio status to be set
 * @param GpioDbgIndx index of the GPIO debug index
 * @return bool success. true if GPIO pin was successfuly set, false if not
 */
bool Rte_Dio_set_gpioDbgSt(bool GpioSts, uint8_t GpioDbgIndx)
{
    bool success = false;
    success = BSW_Dio_write_outputGpioSt((BSW_Dio_gpioOutIndxType) GpioDbgIndx, GpioSts);
    return success;
}

void Rte_Dio_runnable_10ms(void)
{
    // parse CAN message
    {
        // get message pointer
        ComCfg_CanMsgDataType* msgPtr = ComCfg_get_canConfig(CAN_MSG_RX_DO_REQUEST);

        if (true == msgPtr->canRdyForParse)
        {
            // parse CAN message
            uint8_t* canData = &msgPtr->canMsg.data.u8[0];
            Dio_can_parse(canData);

            // CAN message was parsed, clear the parsing flag
            ComCfg_clear_flagCanMsgForParse((ComCfg_canMsgIndxType) CAN_MSG_RX_DO_REQUEST);
        }
    }

    // write GPIO output statuses based on CAN message
    {
        for (uint8_t outIndx = 0; outIndx < NUM_OF_GPIO_OUTPUTS; outIndx++)
        {
            // get output PWM request
            uint16_t outPwm = Dio_read_gpioOutReqSt((BSW_Dio_gpioOutIndxType) outIndx);

            // apply output PWM request
            BSW_Dio_write_outputGpioSt((BSW_Dio_gpioOutIndxType) outIndx, outPwm);
        }
    }

    // send CAN message with GPIO state
    {
        static uint32_t cntr_ms = 0;
        cntr_ms += 10;

        if (100 == cntr_ms)
        {
            // determine CAN message pointer index
            uint16_t canMsgIndx = CAN_MSG_TX_DI_STATUS;   // CAN message index as defined in ComCfg.h

            // get CAN message pointer
            ComCfg_CanMsgDataType* msgPtr = ComCfg_get_canConfig(canMsgIndx);

            // create array of GPIO input values
            bool gpioInArr[NUM_OF_INPUT_INDX] = {0};
            for (uint8_t gpioInIndx = 0; gpioInIndx < NUM_OF_INPUT_INDX; gpioInIndx++)
            {
                gpioInArr[gpioInIndx] = Rte_Dio_get_gpioSt(gpioInIndx);
            }

            // compose CAN data
            Dio_can_compose(&msgPtr->canMsg.data.u8[0], &msgPtr->canMsg.MsgID, &gpioInArr[0]);

            // flag for transmit
            ComCfg_set_flagCanMsgForTx((ComCfg_canMsgIndxType) canMsgIndx);

            cntr_ms = 0;
        }
    }
}