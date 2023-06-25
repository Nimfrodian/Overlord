#ifndef CTRL_GPIO_H
#define CTRL_GPIO_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"

static const uint8_t CTLR_GPIO_TASK_DELAY_TIME_MS = 1;  // task delay time
static const uint8_t DEBOUNCE_TIME_MS = CTLR_GPIO_TASK_DELAY_TIME_MS * 10;  // debounce time in milliseconds
static const uint8_t NUM_OF_SAMPLES_FOR_STATE_CHANGE = 0x1;   // number of consecutive samples that must be of the same value for a state change to occur. Must be lower than 0xF

typedef enum
{
    GPIO_IN_INDX_0,
    GPIO_IN_INDX_1,
    GPIO_IN_INDX_2,
    GPIO_IN_INDX_3,
    GPIO_IN_INDX_4,
    GPIO_IN_INDX_5,
    GPIO_IN_INDX_6,
    GPIO_IN_INDX_7,
    NUM_OF_GPIO_INPUTS
} ctrlGpio_gpioInIndxType;

const gpio_num_t GPIO_IN[NUM_OF_GPIO_INPUTS] =
{
    GPIO_NUM_26,
    GPIO_NUM_25,
    GPIO_NUM_36,
    GPIO_NUM_39,
    GPIO_NUM_35,
    GPIO_NUM_34,
    GPIO_NUM_32,
    GPIO_NUM_33,
};

typedef enum
{
    GPIO_OUT_INDX_0,
    GPIO_OUT_INDX_1,
    GPIO_OUT_INDX_2,
    GPIO_OUT_INDX_3,
    GPIO_OUT_INDX_4,
    GPIO_OUT_INDX_5,
    NUM_OF_GPIO_OUTPUTS
} ctrlGpio_gpioOutIndxType;

const gpio_num_t GPIO_OUT[NUM_OF_GPIO_OUTPUTS] =
{
    GPIO_NUM_15,
    GPIO_NUM_2,
    GPIO_NUM_0,
    GPIO_NUM_18,
    GPIO_NUM_19,
    GPIO_NUM_21,
};

typedef enum
{
    GPIO_OUT_DEBUG_INDX_0 = 0,
    GPIO_OUT_DEBUG_INDX_1,
    GPIO_OUT_DEBUG_NUM_OF_INDX,
} ctrlGpio_gpioOutDbgIndxType;

const gpio_num_t GPIO_OUT_DEBUG[GPIO_OUT_DEBUG_NUM_OF_INDX] =
{
    GPIO_NUM_12,
    GPIO_NUM_13,
};

typedef enum
{
    GPIO_IN_SEL_INDX_0,
    GPIO_IN_SEL_INDX_1,
    NUM_OF_GPIO_IN_SEL
} ctrlGpio_gpioInSelIndxType;

const gpio_num_t GPIO_IN_SEL[NUM_OF_GPIO_IN_SEL] =
{
    GPIO_NUM_27,
    GPIO_NUM_14,
};

typedef enum
{
    // indexes are sorted in a way that allows easier indexing on hardware level
    INPUT_INDX_05 = 0,      ///< 00
    INPUT_INDX_00,          ///< 01
    INPUT_INDX_16,          ///< 02
    INPUT_INDX_21,          ///< 03
    INPUT_INDX_29,          ///< 04
    INPUT_INDX_24,          ///< 05
    INPUT_INDX_13,          ///< 06
    INPUT_INDX_08,          ///< 07

    INPUT_INDX_06,          ///< 08
    INPUT_INDX_03,          ///< 09
    INPUT_INDX_19,          ///< 10
    INPUT_INDX_22,          ///< 11
    INPUT_INDX_30,          ///< 12
    INPUT_INDX_27,          ///< 13
    INPUT_INDX_14,          ///< 14
    INPUT_INDX_11,          ///< 15

    INPUT_INDX_07,          ///< 16
    INPUT_INDX_01,          ///< 17
    INPUT_INDX_17,          ///< 18
    INPUT_INDX_23,          ///< 19
    INPUT_INDX_31,          ///< 20
    INPUT_INDX_25,          ///< 21
    INPUT_INDX_15,          ///< 22
    INPUT_INDX_09,          ///< 23

    INPUT_INDX_04,          ///< 24
    INPUT_INDX_02,          ///< 25
    INPUT_INDX_18,          ///< 26
    INPUT_INDX_20,          ///< 27
    INPUT_INDX_28,          ///< 28
    INPUT_INDX_26,          ///< 29
    INPUT_INDX_12,          ///< 30
    INPUT_INDX_10,          ///< 31
    NUM_OF_INPUT_INDX
} CtrlGpio_inputIndxType;

void CtrlGpio_init(void);
void CtrlGpio_read(void* param);
void CtrlGpio_write(void* param);

bool GpioCtrl_get_gpioSts(uint8_t gpioIndx);
bool GpioCtrl_set_gpioDbgSts(bool GpioSts, uint8_t GpioDbgIndx);


#endif