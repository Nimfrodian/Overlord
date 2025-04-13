#ifndef DIOM_RTDB_VARS_H
#define DIOM_RTDB_VARS_H

#include "rtdb.h"

#define DIOM_TI_US_MIN_TASK_TIME_U32 ((uint32_t) 10)
#define DIOM_TI_US_MAX_TASK_TIME_U32 ((uint32_t) 1000)
#define DIOM_TI_US_MIN_COOLDOWN_TIME (DIOM_TI_US_MAX_TASK_TIME_U32 * 4)
#define DIOM_TI_US_MAX_COOLDOWN_TIME (DIOM_TI_US_MIN_COOLDOWN_TIME * 10)
/*
 * note; min, default and max can be left empty if their value is not limited or relevant
 * [varType]    [arrName]                              [SIZE (optional)]; ///< [unit]  [min]  [default]  [max]    [comment]
 */
tU32S    diom_ti_cooldownPeriod_U32;      ///< [us]  [DIOM_TI_US_MIN_COOLDOWN_TIME]  [5000]  [DIOM_TI_US_MAX_COOLDOWN_TIME]   [Time in us for how long the signal has to settle before change to it can be registered again.]
tU32S    diom_ti_taskTime_U32;            ///< [us]  [DIOM_TI_US_MIN_TASK_TIME_U32]  [100]   [DIOM_TI_US_MAX_TASK_TIME_U32]   [Time in us between task execution]
tU32S    diom_ti_inputStates_U32;         ///< []    [0]                             [0]     [0xFFFFFFFF]                     [States of the inputs as 32 bit mask]

#endif