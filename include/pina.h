/*
*   pins abstraction file (GPIO)
*
*
*/
#pragma once

#include "driver/gpio.h"
#include "errh.h"

#define PINA_API_INIT_U32                  ((uint32_t) 1)

typedef struct
{
} tPINA_INITDATA_STR;

typedef enum
{
    PINA_MUX_IN_0 = GPIO_NUM_6,     ///< Multiplexor 1 output pin 0
    PINA_MUX_IN_1 = GPIO_NUM_7,     ///< Multiplexor 1 output pin 1
    PINA_MUX_IN_2 = GPIO_NUM_15,    ///< Multiplexor 2 output pin 0
    PINA_MUX_IN_3 = GPIO_NUM_16,    ///< Multiplexor 2 output pin 1
    PINA_MUX_IN_4 = GPIO_NUM_35,    ///< Multiplexor 3 output pin 0
    PINA_MUX_IN_5 = GPIO_NUM_36,    ///< Multiplexor 3 output pin 1
    PINA_MUX_IN_6 = GPIO_NUM_37,    ///< Multiplexor 4 output pin 0
    PINA_MUX_IN_7 = GPIO_NUM_38,    ///< Multiplexor 4 output pin 1

    PINA_MUX_OUT_0 = GPIO_NUM_3,    ///< Multiplexors select pin 0
    PINA_MUX_OUT_1 = GPIO_NUM_46,   ///< Multiplexors select pin 1

    PINA_CAN_TX = GPIO_NUM_5,       ///< CAN TX pin
    PINA_CAN_RX = GPIO_NUM_4,       ///< CAN RX pin

    PINA_MB_1_TX = GPIO_NUM_18,     ///< RS485 TX pin
    PINA_MB_1_RX = GPIO_NUM_17,     ///< RS485 RX pin
    PINA_MB_1_DE = GPIO_NUM_11,     ///< RS485 DE pin

    PINA_MB_2_TX = GPIO_NUM_10,     ///< RS485 TX pin
    PINA_MB_2_RX = GPIO_NUM_9,      ///< RS485 RX pin
    PINA_MB_2_DE = GPIO_NUM_12,     ///< RS485 DE pin

    PINA_LED_0 = GPIO_NUM_13,       ///< LED on connection board
    PINA_LED_1 = GPIO_NUM_14,       ///< LED on connection board
    PINA_LED_2 = GPIO_NUM_21,       ///< LED on connection board

    // INPUTS
    PINA_IN_NUM_0 = GPIO_NUM_38,
} PINA_nr_GPIO_NUM_E;

/**
 * @brief Function initializes pin abstraction handling module
 * @param
 * @return (void)
 */
void pina_init(tPINA_INITDATA_STR* PinaCfg);

/**
 * @brief Function sets GPIO output to desired value
 * @param GpioNum GPIO pin to set value to
 * @param Value value to apply
 * @return (void)
 */
void pina_setGpioLevel(PINA_nr_GPIO_NUM_E GpioNum, bool Value);

/**
 * @brief Function gets GPIO output to desired value
 * @param GpioNum GPIO pin to get value from
 * @return (void)
 */
bool pina_getGpioLevel(PINA_nr_GPIO_NUM_E GpioNum);

/**
 * @brief Function sets GPIO pin as output
 * @param GpioNum GPIO number
 * @return (void)
 */
void pina_setGpioAsOutput(PINA_nr_GPIO_NUM_E GpioNum);

/**
 * @brief Function sets GPIO pin as input
 * @param GpioNum GPIO number
 * @return (void)
 */
void pina_setGpioAsInput(PINA_nr_GPIO_NUM_E GpioNum);

/**
 * @brief Function sets GPIO pin for interrupt service
 * @param GpioNum GPIO number
 * @param func pointer to a function to be executed on GPIO interrupt
 * @return (void)
 */
void pina_setInterruptService(PINA_nr_GPIO_NUM_E GpioNum, void (*func)(void*arg));