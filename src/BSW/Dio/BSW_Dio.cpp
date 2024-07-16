#include "Dio_Cfg.h"

static uint8_t _gpioState[NUM_OF_INPUT_INDX] = {0};                // holds end result
static uint8_t _gpioCntr[NUM_OF_INPUT_INDX] = {0};              // increments or decrements based on GPIO state.
static uint8_t _debounceTimer_ms[NUM_OF_INPUT_INDX] = {0};      // holds debounce timer. Decrements by task execution period each call, if not 0
static const uint8_t _sortedGpioIndxs[NUM_OF_INPUT_INDX] =             // sorted array indexes. Looping _gpioState using this array as index will result in proper reading from GPIO
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


void BSW_Dio_init(void)
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

    // create reading task
    xTaskCreatePinnedToCore(
    BSW_Dio_read,    // Function that should be called
    "BSW_Dio_read",   // Name of the task (for debugging)
    8192,            // Stack size (bytes)
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
static void checkInputForChange(BSW_Dio_inputIndxType InputIndx, uint8_t GpioSt)
{
    // proceed if this input is not on debounce timer
    if (0 == _debounceTimer_ms[InputIndx])
    {
        // delete the appropriate counter. 0x0F bits are "1" counter, 0xF0 bits are "0" counter
        _gpioCntr[InputIndx] &= (0x0F << (4 * !GpioSt));

        // if gpio state is different than saved
        if (GpioSt != _gpioState[InputIndx])
        {
            // if previous state is different and not enough samples are yet taken then increment the appropriate counter
            if (NUM_OF_SAMPLES_FOR_STATE_CHANGE > (0x0F & (_gpioCntr[InputIndx] >> (4 * !GpioSt))))
            {
                _gpioCntr[InputIndx] += 0x01 << (4 * !GpioSt);
            }

            // save the new state as enough samples were taken and add the debounce time
            if (NUM_OF_SAMPLES_FOR_STATE_CHANGE == (0x0F & (_gpioCntr[InputIndx] >> (4 * !GpioSt))))
            {
                // save the new state
                _gpioState[InputIndx] = GpioSt;

                // set the debounce timer
                _debounceTimer_ms[InputIndx] = DEBOUNCE_TIME_MS;
            }
        }
    }
    else // debounce time is active and decrement the timer by task delay
    {
        _debounceTimer_ms[InputIndx] -= CTLR_GPIO_TASK_DELAY_TIME_MS;
    }
}

void BSW_Dio_read(void* param)
{
    while (1)
    {
        // selector index. 0x00-GPIOCTLR_SELECTOR_MAX_VAL
        static uint8_t selIndx = 0;

        // sample counter. 0x00-NUM_OF_SAMPLES_FOR_STATE_CHANGE. There are NUM_OF_SAMPLES_FOR_STATE_CHANGE per selector increment
        static uint8_t sampleCntr = 0;

        // read all gpio inputs
        uint8_t currGpioSt[NUM_OF_GPIO_INPUTS] = {0};  // holds current GPIO status
        for (uint8_t gpioInIndx = 0; gpioInIndx < NUM_OF_GPIO_INPUTS; gpioInIndx++)
        {
            currGpioSt[gpioInIndx] = gpio_get_level(GPIO_IN[gpioInIndx]);   // read the current value
        }

        // determine if value needs to be updated
        {
            const uint8_t numOfMuxUnits = NUM_OF_GPIO_INPUTS; // number of multiplexer units
            for (uint8_t gpioInIndx = 0; gpioInIndx < numOfMuxUnits; gpioInIndx++)
            {
                checkInputForChange((BSW_Dio_inputIndxType) ((selIndx * numOfMuxUnits) + gpioInIndx), currGpioSt[gpioInIndx]);
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

uint8_t BSW_Dio_read_inputGpioSt(BSW_Dio_inputIndxType GpioIndx)
{
    uint8_t gpioSt = 0;
    if (NUM_OF_INPUT_INDX > GpioIndx)
    {
        gpioSt = _gpioState[_sortedGpioIndxs[GpioIndx]];
    }
    else
    {
        // TODO: REPORT_ERROR
    }
    return gpioSt;
}