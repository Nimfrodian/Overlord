#ifndef DIOM_RTDB_VARS_H
#define DIOM_RTDB_VARS_H

#include "rtdb.h"

#define DIOM_TI_US_MIN_TASK_TIME_U32 ((uint32_t) 10)        //   10 us between readings
#define DIOM_TI_US_DEF_TASK_TIME_U32 ((uint32_t) 500)       //   500 us between readings
#define DIOM_TI_US_MAX_TASK_TIME_U32 ((uint32_t) 100000)    //  100 ms between readings
#define DIOM_TI_US_MIN_COOLDOWN_TIME_U32 ((uint32_t) 10)        //  10 us
#define DIOM_TI_US_DEF_COOLDOWN_TIME_U32 ((uint32_t) 3000)      //   3 ms
#define DIOM_TI_US_MAX_COOLDOWN_TIME_U32 ((uint32_t) 100000)    // 100 ms
/*
 * note; min, default and max can be left empty if their value is not limited or relevant
 * [varType]    [arrName]                              [SIZE (optional)]; ///< [unit]  [min]  [default]  [max]    [comment]
 */
tU32S    rtdb_diom_ti_us_cooldownPeriod_U32;      ///< [us]  [DIOM_TI_US_MIN_COOLDOWN_TIME_U32]  [DIOM_TI_US_DEF_COOLDOWN_TIME_U32]  [DIOM_TI_US_MAX_COOLDOWN_TIME_U32]   [Time in us for how long the signal has to settle before change to it can be registered again.]
tU32S    rtdb_diom_ti_us_taskTime_U32;            ///< [us]  [DIOM_TI_US_MIN_TASK_TIME_U32]      [DIOM_TI_US_DEF_TASK_TIME_U32]      [DIOM_TI_US_MAX_TASK_TIME_U32]       [Time in us between task execution]
tU32S    rtdb_diom_x_inputStates_U32;             ///< []    [0]                                 [0]                                 [0xFFFFFFFF]                         [States of the inputs as 32 bit mask]

#endif