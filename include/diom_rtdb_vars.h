#ifndef DIOM_RTDB_VARS_H
#define DIOM_RTDB_VARS_H

#include "rtdb.h"

/*
 * note; min, default and max can be left empty if their value is not limited or relevant
 * [varType]    [arrName]                              [SIZE (optional)]; ///< [unit]  [min]  [default]  [max]    [comment]
 */
tU32S    diom_ti_cooldownPeriod_U32;      ///< [us]  [0]  [2000]  [10000]    [Time in us for how long the signal has to settle before change to it can be registered again.]
tU32S    diom_ti_taskTime_U32;            ///< [us]  [0]  [500]   [1000]     [Time in us between task execution]
tU32S    diom_ti_inputStates_U32;         ///< []    [0]  [0]     [0xFFFF]   [States of the inputs as 32 bit mask]

#endif