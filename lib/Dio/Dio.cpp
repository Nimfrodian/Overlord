#include "Dio.h"

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
void Dio_can_compose(uint8_t* CanDataPtr, uint32_t* CanIdPtr, uint8_t* GpioStArr)
{
    // clear existing CAN data
    for (uint8_t i = 0; i < 8; i++)
    {
        CanDataPtr[i] = 0;
    }

    // fill CAN message with GPIO input states
    for (uint8_t indx = 0; indx < 32; indx++)
    {
        CanDataPtr[indx / 8] |= GpioStArr[indx] << (indx % 8u);
    }

    *CanIdPtr = 0x150;
}