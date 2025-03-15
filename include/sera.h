/*
*   UART abstraction file
*   used to abstract away UART related names
*
*/
#pragma once

#include "driver/uart.h"
#include "stdarg.h"
#include "errh.h"

#define SERA_API_INIT_U32                  ((uint32_t) 1)
#define SERA_API_WRITE_U32                 ((uint32_t) 2)
#define SERA_API_READ_U32                  ((uint32_t) 3)

typedef struct
{
    uint32_t nr_moduleId_U32;       ///< ID of the module
} tSERA_INITDATA_STR;

/**
 * @brief Function initializes Serial Communication
 * @param void
 * @return (void)
 */
void sera_init(tSERA_INITDATA_STR* SeraCfg);

/**
 * @brief Function prints text to UART
 * @param text to be printed
 * @param ... additional arguments, akin to sprintf
 * @return number of bytes written
 */
uint32_t sera_print(const char* Text, ...);

/**
 * @brief Function returns data from UART buffer
 * @param buffer pointer to a buffer
 * @return
 */
uint32_t sera_read(char* Buffer, uint32_t BuffLength);