/*
*
*   Time handling module
*
*/
#pragma once

#include "stdint.h"
#include "vars.h"

#define TIMH_API_INIT_U32                  ((uint32_t) 1)
#define TIMH_API_CAN_PARSE_U32             ((uint32_t) 2)
#define TIMH_API_READ_SYS_TI_U32           ((uint32_t) 3)
#define TIMH_API_DELAY_US_U32              ((uint32_t) 4)

#define TIMH_ERR_WRONG_CAN_ID_U32          ((uint32_t) 1)

typedef struct
{
    int64_t (*timh_ti_us_sysTimeFunc_pfS64)(void);   ///< pointer to system time function
} tTIMH_INITDATA_STR;

/**
 * @brief Function initialized timh module
 * @param InitData_pstr
 * @return (void)
 */
void timh_init(tTIMH_INITDATA_STR* InitData_pstr);

/**
 * @brief Function parses CAN messages related to timh module. Message should arrive at least every second
 * @param DataPtr pointer to data to be parsed
 * @param MsgIdPtr pointer to message ID
 * @return (void)
 */
void timh_canMsgParse_ev(uint8_t* DataPtr, uint32_t* MsgIdPtr);

/**
 * @brief Function returns current (real) time
 * @param void
 * @return copy of tVARS_TIMEDATA_STR with current real world time
 */
tVARS_TIMEDATA_STR timh_ti_readCurrentGlobalTime(void);

/**
 * @brief Function returns system time since reset in us
 * @param void
 * @return system time in us since reset
 */
int64_t timh_ti_us_readSystemTime_S64(void);

/**
 * @brief Function waits in a while loop for a given time in us
 * @param int64_t delay_us time to wait in us
 * @return void
 */
void timh_delayMicroseconds(int64_t delay_us);