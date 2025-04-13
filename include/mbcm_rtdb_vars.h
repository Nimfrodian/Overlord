#ifndef MBCM_RTDB_VARS_H
#define MBCM_RTDB_VARS_H

#include "rtdb.h"

#define MBCM_TI_US_MIN_TASK_TIME_RELAY_U32 ((uint32_t) 10)          //  10 us between queries
#define MBCM_TI_US_DEF_TASK_TIME_RELAY_U32 ((uint32_t) 2000)        //   2 ms between queries
#define MBCM_TI_US_MAX_TASK_TIME_RELAY_U32 ((uint32_t) 10000)       //  10 ms between queries


#define MBCM_TI_US_MIN_TASK_TIME_PWRMETER_U32 ((uint32_t) 10)       //  10 us between queries
#define MBCM_TI_US_DEF_TASK_TIME_PWRMETER_U32 ((uint32_t) 100000)   // 100 ms between queries
#define MBCM_TI_US_MAX_TASK_TIME_PWRMETER_U32 ((uint32_t) 1000000)  //   1 s between queries

/*
 * note; min, default and max can be left empty if their value is not limited or relevant
 * [varType]    [arrName]                              [SIZE (optional)]; ///< [unit]  [min]  [default]  [max]    [comment]
 */
tU32S    mbcm_ti_us_taskPeriodRelay_U32;         ///< [us]  [MBCM_TI_US_MIN_TASK_TIME_RELAY_U32]     [MBCM_TI_US_DEF_TASK_TIME_RELAY_U32]     [MBCM_TI_US_MAX_TASK_TIME_RELAY_U32]      [Time in us between Modbus messages for Relay modules]
tU32S    mbcm_ti_us_taskPeriodPwrMeter_U32;      ///< [us]  [MBCM_TI_US_MIN_TASK_TIME_PWRMETER_U32]  [MBCM_TI_US_DEF_TASK_TIME_PWRMETER_U32]  [MBCM_TI_US_MAX_TASK_TIME_PWRMETER_U32]   [Time in us between Modbus messages for Power Meter modules]

tU32S    mbcm_x_desiredRelayStates_U32;          ///< []    [0]                                      [0]                                      [0xFFFFFFFF]                              [Desired relay states as 32 bit mask]

#endif