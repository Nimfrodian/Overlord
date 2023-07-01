#include "GpioControl.h"

/**
 * @brief Function returns the state of the GPIO pin if GpioIndx is in the correct range
 * @param GpioIndx GPIO index for which to get value
 * @return GPIO state
 */
bool Rte_GpioCtrl_get_gpioSts(uint8_t GpioIndx)
{
    bool success = false;
    success = CtrlGpio_read_gpioSts(GpioIndx);
    return success;
}

/**
 * @brief Function sets the GPIO debug pin to desired state for debugging purposes
 * @param GpioSts gpio status to be set
 * @param GpioDbgIndx index of the GPIO debug index
 * @return bool success. true if GPIO pin was successfuly set, false if not
 */
bool Rte_GpioCtrl_set_gpioDbgSts(bool GpioSts, uint8_t GpioDbgIndx)
{
    bool success = false;
    success = CtrlGpio_write_gpioSts(GpioDbgIndx, GpioSts);
    return success;
}

// TODO: Add a runnable that will set GPIO states based on a CAN message