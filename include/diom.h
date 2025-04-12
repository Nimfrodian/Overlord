/*
*   Digital input output module
*
*
*/
#pragma once

#include "pina.h"
#include "rtdb.h"
#include "errh.h"

#define DIOM_API_INIT_U32                  ((uint32_t) 1)

#define DIOM_NUM_OF_MUX_U32         ((uint32_t) 4)   ///< number of multiplexors
#define DIOM_NUM_OF_MUX_INPUTS_U32  ((uint32_t) 8)   ///< number of inputs per multiplexor

typedef struct
{
    uint32_t nr_moduleId_U32;       ///< ID of the module
} tDIOM_INITDATA_STR;

typedef struct
{
    bool output_B;
    bool lastReadState_B;
    tU32 ti_us_cooldownTime_U32;
} tDIOM_GPIODATA_STR;

typedef enum
{
    DIOM_MUX_IN_PIN_0 = PINA_MUX_IN_0,  ///< Multiplexor 1 output pin 0
    DIOM_MUX_IN_PIN_1 = PINA_MUX_IN_1,  ///< Multiplexor 1 output pin 1
    DIOM_MUX_IN_PIN_2 = PINA_MUX_IN_2,  ///< Multiplexor 2 output pin 0
    DIOM_MUX_IN_PIN_3 = PINA_MUX_IN_3,  ///< Multiplexor 2 output pin 1
    DIOM_MUX_IN_PIN_4 = PINA_MUX_IN_4,  ///< Multiplexor 3 output pin 0
    DIOM_MUX_IN_PIN_5 = PINA_MUX_IN_5,  ///< Multiplexor 3 output pin 1
    DIOM_MUX_IN_PIN_6 = PINA_MUX_IN_6,  ///< Multiplexor 4 output pin 0
    DIOM_MUX_IN_PIN_7 = PINA_MUX_IN_7,  ///< Multiplexor 4 output pin 1
} tDIOM_MUXINPUTPIN_E;
#define DIOM_MUX_NUM_OF_INPUTS_U32 ((uint32_t) 8)   ///< number of input pins on ESP32 from multiplexors

typedef enum
{
    DIOM_MUX_OUT_PIN_0 = PINA_MUX_OUT_0,    ///< Multiplexors select pin 0
    DIOM_MUX_OUT_PIN_1 = PINA_MUX_OUT_1,    ///< Multiplexors select pin 1
} tDIOM_MUXOUTPUTPIN_E;
#define DIOM_MUX_NUM_OF_OUTPUTS_U32 ((uint32_t) 2)

void diom_init(tDIOM_INITDATA_STR* DiomCfg);