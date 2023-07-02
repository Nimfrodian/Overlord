#include "Dio.h"

static uint16_t _reqDioOutSt[NUM_OF_GPIO_OUTPUTS] = {0}; // variable holds requested output state of GPIO pin

void Dio_init(void)
{

}

/**
 * @brief Function fills CAN message data with GPIO input states. Message size 8 bytes
 * @param CanData pointer to CAN message data
 * @param CanId pointer to CAN message ID variable
 * @param GpioStArr array of GPIO input states. Must be of size NUM_OF_INPUT_INDX
 * @return (void)
 */
void Dio_can_compose(uint8_t* CanDataPtr, uint32_t* CanIdPtr, bool* GpioStArr)
{
    // clear existing CAN data
    for (uint8_t i = 0; i < 8; i++)
    {
        CanDataPtr[i] = 0;
    }

    // fill CAN message with GPIO input states
    for (uint8_t indx = 0; indx < NUM_OF_INPUT_INDX; indx++)
    {
        CanDataPtr[indx / 8] |= GpioStArr[indx] << (indx % 8u);
    }

    *CanIdPtr = 0x150;
}

/**
 * @brief Function parses CAN data request for GPIO out state
 * @param CanDataPtr pointer to CAN message data
 * @return (void)
 */
void Dio_can_parse(uint8_t* CanDataPtr)
{
    // copy request from CAN to each digital output state
    uint16_t tempIndx0 = 0x00;
    tempIndx0 = ((CanDataPtr[1] & 0x03) << 8) | (CanDataPtr[0] >> 0);
    uint16_t tempIndx1 = 0x00;
    tempIndx1 = ((CanDataPtr[2] & 0x0F) << 6) | (CanDataPtr[1] >> 2);
    uint16_t tempIndx2 = 0x00;
    tempIndx2 = ((CanDataPtr[3] & 0x3F) << 4) | (CanDataPtr[2] >> 4);
    uint16_t tempIndx3 = 0x00;
    tempIndx3 = ((CanDataPtr[4] & 0xFF) << 2) | (CanDataPtr[3] >> 6);
    uint16_t tempIndx4 = 0x00;
    tempIndx4 = ((CanDataPtr[6] & 0x03) << 8) | (CanDataPtr[5] >> 0);
    uint16_t tempIndx5 = 0x00;
    tempIndx5 = ((CanDataPtr[7] & 0x0F) << 6) | (CanDataPtr[6] >> 2);

    _reqDioOutSt[GPIO_OUT_INDX_0] = tempIndx0;
    _reqDioOutSt[GPIO_OUT_INDX_1] = tempIndx1;
    _reqDioOutSt[GPIO_OUT_INDX_2] = tempIndx2;
    _reqDioOutSt[GPIO_OUT_INDX_3] = tempIndx3;
    _reqDioOutSt[GPIO_OUT_INDX_4] = tempIndx4;
    _reqDioOutSt[GPIO_OUT_INDX_5] = tempIndx5;
}

/**
 * @brief Function returns 10bit resolution of PWM request on given GPIO pin
 * @param outIndx output GPIO index for which the PWM request should be returned
 * @return 10bit PWM request for the given pin
 */
uint16_t Dio_read_gpioOutReqSt(BSW_Dio_gpioOutIndxType outIndx)
{
    return _reqDioOutSt[outIndx];
}