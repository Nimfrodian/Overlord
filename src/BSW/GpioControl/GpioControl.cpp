#include "GpioControl.h"

static bool _gpioState[NUM_OF_INPUT_INDX] = {0};            // holds end result
static bool _gpioOutState[NUM_OF_GPIO_OUTPUTS] = {0};       // holds outputs for GPIO out abstraction
static uint8_t _gpioCntr[NUM_OF_INPUT_INDX] = {0};          // increments or decrements based on GPIO state.
static uint8_t _debounceTimer_ms[NUM_OF_INPUT_INDX] = {0};  // holds debounce timer. Decrements by task execution period each call, if not 0
const uint8_t _sortedGpioIndxs[NUM_OF_INPUT_INDX] =         // sorted array indexes. Looping _gpioState using this array as index will result in proper reading from GPIO
{
    INPUT_INDX_00,
    INPUT_INDX_01,
    INPUT_INDX_02,
    INPUT_INDX_03,
    INPUT_INDX_04,
    INPUT_INDX_05,
    INPUT_INDX_06,
    INPUT_INDX_07,
    INPUT_INDX_08,
    INPUT_INDX_09,
    INPUT_INDX_10,
    INPUT_INDX_11,
    INPUT_INDX_12,
    INPUT_INDX_13,
    INPUT_INDX_14,
    INPUT_INDX_15,
    INPUT_INDX_16,
    INPUT_INDX_17,
    INPUT_INDX_18,
    INPUT_INDX_19,
    INPUT_INDX_20,
    INPUT_INDX_21,
    INPUT_INDX_22,
    INPUT_INDX_23,
    INPUT_INDX_24,
    INPUT_INDX_25,
    INPUT_INDX_26,
    INPUT_INDX_27,
    INPUT_INDX_28,
    INPUT_INDX_29,
    INPUT_INDX_30,
    INPUT_INDX_31,
};

void CtrlGpio_init(void)
{
    // set all gpio input pins as INPUT
    for (uint8_t gpioInIndx = 0; gpioInIndx < NUM_OF_GPIO_INPUTS; gpioInIndx++)
    {
        gpio_set_direction(GPIO_IN[gpioInIndx], GPIO_MODE_INPUT);
    }

    // set all gpio selection pins as OUTPUT
    for (uint8_t gpioInSelIndx = 0; gpioInSelIndx < NUM_OF_GPIO_IN_SEL; gpioInSelIndx++)
    {
        gpio_reset_pin(GPIO_IN_SEL[gpioInSelIndx]);
        gpio_set_direction(GPIO_IN_SEL[gpioInSelIndx], GPIO_MODE_OUTPUT);
    }

    // reset all output pins functions so they can be set as OUTPUT and set them as OUTPUT
    for (uint8_t gpioOutIndx = 0; gpioOutIndx < NUM_OF_GPIO_OUTPUTS; gpioOutIndx++)
    {
        gpio_reset_pin(GPIO_OUT[gpioOutIndx]);
        gpio_set_direction(GPIO_OUT[gpioOutIndx], GPIO_MODE_OUTPUT);
    }

    // reset all debug output pins functions so they can be set as OUTPUT and set them as OUTPUT
    for (uint8_t gpioOutDbgIndx = 0; gpioOutDbgIndx < GPIO_OUT_DEBUG_NUM_OF_INDX; gpioOutDbgIndx++)
    {
        gpio_reset_pin(GPIO_OUT_DEBUG[gpioOutDbgIndx]);
        gpio_set_direction(GPIO_OUT_DEBUG[gpioOutDbgIndx], GPIO_MODE_OUTPUT);
    }

    // create reading task
    xTaskCreatePinnedToCore(
    CtrlGpio_read,    // Function that should be called
    "CtrlGpio_read",   // Name of the task (for debugging)
    8192,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL,            // Task handle
    0                // run on core 0

    );

    // create writing task
    xTaskCreatePinnedToCore(
    CtrlGpio_write,     // Function that should be called
    "CtrlGpio_write",   // Name of the task (for debugging)
    1024,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL,            // Task handle
    0                // run on core 0
    );
}

/**
 * @brief Function checks if input variable needs to be changed as part of debouncing
 * @param inputIndx input index of the input variable. Goes from 0 to NUM_OF_INPUT_INDX
 * @param gpioSt the current state of the input variable as read from GPIO
 * @return (void)
 */
static void checkInputForChange(CtrlGpio_inputIndxType inputIndx, bool gpioSt)
{
    // proceed if this input is not on debounce timer
    if (0 == _debounceTimer_ms[inputIndx])
    {
        // delete the appropriate counter. 0x0F bits are "1" counter, 0xF0 bits are "0" counter
        _gpioCntr[inputIndx] &= (0x0F << (4 * !gpioSt));

        // if gpio state is different than saved
        if (gpioSt != _gpioState[inputIndx])
        {
            // if previous state is different and not enough samples are yet taken then increment the appropriate counter
            if (NUM_OF_SAMPLES_FOR_STATE_CHANGE > (0x0F & (_gpioCntr[inputIndx] >> (4 * !gpioSt))))
            {
                _gpioCntr[inputIndx] += 0x01 << (4 * !gpioSt);
            }

            // save the new state as enough samples were taken and add the debounce time
            if (NUM_OF_SAMPLES_FOR_STATE_CHANGE == (0x0F & (_gpioCntr[inputIndx] >> (4 * !gpioSt))))
            {
                // save the new state
                _gpioState[inputIndx] = gpioSt;

                // set the debounce timer
                _debounceTimer_ms[inputIndx] = DEBOUNCE_TIME_MS;
            }
        }
    }
    else // debounce time is active and decrement the timer by task delay
    {
        _debounceTimer_ms[inputIndx] -= CTLR_GPIO_TASK_DELAY_TIME_MS;
    }
}

void CtrlGpio_read(void* param)
{
    while (1)
    {
        // selector index. 0x00-GPIOCTLR_SELECTOR_MAX_VAL
        static uint8_t selIndx = 0;

        // sample counter. 0x00-NUM_OF_SAMPLES_FOR_STATE_CHANGE. There are NUM_OF_SAMPLES_FOR_STATE_CHANGE per selector increment
        static uint8_t sampleCntr = 0;

        // read all gpio inputs
        bool currGpioSt[NUM_OF_GPIO_INPUTS] = {0};  // holds current GPIO status
        for (uint8_t gpioInIndx = 0; gpioInIndx < NUM_OF_GPIO_INPUTS; gpioInIndx++)
        {
            currGpioSt[gpioInIndx] = gpio_get_level(GPIO_IN[gpioInIndx]);   // read the current value
        }

        // determine if value needs to be updated
        {
            const uint8_t numOfMuxUnits = NUM_OF_GPIO_INPUTS; // number of multiplexer units
            for (uint8_t gpioInIndx = 0; gpioInIndx < numOfMuxUnits; gpioInIndx++)
            {
                checkInputForChange((CtrlGpio_inputIndxType) ((selIndx * numOfMuxUnits) + gpioInIndx), currGpioSt[gpioInIndx]);
            }
        }

        // increment sample counter and selector index
        {
            // increment sample counter
            sampleCntr++;

            // if enough samples have been taken at this selector index then reset sample counter
            if (NUM_OF_SAMPLES_FOR_STATE_CHANGE <= sampleCntr)
            {
                sampleCntr = 0;

                // increment selector index
                selIndx++;

                // rollover selector index
                if ((0x01 << NUM_OF_GPIO_IN_SEL) <= selIndx)
                {
                    selIndx = 0;
                }
            }
        }

        // set new selector value
        {
            for (uint8_t gpioSelIndx = 0; gpioSelIndx < NUM_OF_GPIO_IN_SEL; gpioSelIndx++)
            {
                gpio_set_level(GPIO_IN_SEL[gpioSelIndx], 0 < (selIndx & (0x01 + gpioSelIndx)));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(CTLR_GPIO_TASK_DELAY_TIME_MS));
    }

    // delete task if illegal state was reached
    vTaskDelete( NULL );
}


void CtrlGpio_write(void* param)
{
    while (1)
    {
        // temp
        static uint8_t temp = 0;
        temp++;
        for (uint8_t gpioOutIndx = 0; gpioOutIndx < NUM_OF_GPIO_OUTPUTS; gpioOutIndx++)
        {
            _gpioOutState[gpioOutIndx] = 0 < (temp & (0x01 << gpioOutIndx));
        }

        // end temp

        // set new GPIO values
        for (uint8_t gpioOutIndx = 0; gpioOutIndx < NUM_OF_GPIO_OUTPUTS; gpioOutIndx++)
        {
            gpio_set_level(GPIO_OUT[gpioOutIndx], _gpioOutState[gpioOutIndx]);
        }

        vTaskDelay(pdMS_TO_TICKS(CTLR_GPIO_TASK_DELAY_TIME_MS));
    }
    vTaskDelete( NULL );
}

/**
 * @brief Function returns the state of the GPIO pin if GpioIndx is in the correct range
 * @param GpioIndx GPIO index for which to get value
 * @return GPIO state
 */
bool GpioCtrl_get_gpioSts(uint8_t GpioIndx)
{
    uint8_t indx = 0;

    // check if index is valid
    if (GpioIndx < NUM_OF_INPUT_INDX)
    {
        indx = GpioIndx;
    }

    // return GPIO state based on HW indexing
    return _gpioState[_sortedGpioIndxs[indx]];
}

/**
 * @brief Function sets the GPIO debug pin to desired state for debugging purposes
 * @param GpioSts gpio status to be set
 * @param GpioDbgIndx index of the GPIO debug index
 * @return bool success. true if GPIO pin was successfuly set, false if not
 */
bool GpioCtrl_set_gpioDbgSts(bool GpioSts, uint8_t GpioDbgIndx)
{
    // create return variable
    bool success = false;

    // check if input parameters are OK and set GPIO pin if they are
    if (GPIO_OUT_DEBUG_NUM_OF_INDX > GpioDbgIndx)
    {
        gpio_set_level((gpio_num_t) GpioDbgIndx, GpioSts);
        success = true;
    }

    return success;
}