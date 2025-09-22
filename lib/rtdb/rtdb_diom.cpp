
#include "rtdb.h"

#define DIOM_TI_US_MIN_TASK_TIME_U32 ((uint32_t) 10)        //   10 us between readings
#define DIOM_TI_US_DEF_TASK_TIME_U32 ((uint32_t) 500)       //   500 us between readings
#define DIOM_TI_US_MAX_TASK_TIME_U32 ((uint32_t) 100000)    //  100 ms between readings
#define DIOM_TI_US_MIN_COOLDOWN_TIME_U32 ((uint32_t) 10)        //  10 us
#define DIOM_TI_US_DEF_COOLDOWN_TIME_U32 ((uint32_t) 3000)      //   3 ms
#define DIOM_TI_US_MAX_COOLDOWN_TIME_U32 ((uint32_t) 100000)    // 100 ms

tU32S    rtdb_diom_ti_us_cooldownPeriod_U32;      ///< [us]  [DIOM_TI_US_MIN_COOLDOWN_TIME_U32]  [DIOM_TI_US_DEF_COOLDOWN_TIME_U32]  [DIOM_TI_US_MAX_COOLDOWN_TIME_U32]   [Time in us for how long the signal has to settle before change to it can be registered again.]
tU32S    rtdb_diom_ti_us_taskTime_U32;            ///< [us]  [DIOM_TI_US_MIN_TASK_TIME_U32]      [DIOM_TI_US_DEF_TASK_TIME_U32]      [DIOM_TI_US_MAX_TASK_TIME_U32]       [Time in us between task execution]
tU32S    rtdb_diom_x_inputStates_U32;             ///< []    [0]                                 [0]                                 [0xFFFFFFFF]                         [States of the inputs as 32 bit mask]


void diom_rtdb_init()
{
    rtdb_assign_tU32S(&rtdb_diom_ti_us_cooldownPeriod_U32, RTDB_DIOM_TI_US_COOLDOWNPERIOD_U32, VAR_UNIT_us, DIOM_TI_US_MIN_COOLDOWN_TIME_U32, DIOM_TI_US_DEF_COOLDOWN_TIME_U32, DIOM_TI_US_MAX_COOLDOWN_TIME_U32, "Time in us for how long the signal has to settle before change to it can be registered again.");
    rtdb_assign_tU32S(&rtdb_diom_ti_us_taskTime_U32, RTDB_DIOM_TI_US_TASKTIME_U32, VAR_UNIT_us, DIOM_TI_US_MIN_TASK_TIME_U32, DIOM_TI_US_DEF_TASK_TIME_U32, DIOM_TI_US_MAX_TASK_TIME_U32, "Time in us between task execution");
    rtdb_assign_tU32S(&rtdb_diom_x_inputStates_U32, RTDB_DIOM_X_INPUTSTATES_U32, VAR_UNIT_NONE, 0, 0, 0xFFFFFFFF, "States of the inputs as 32 bit mask");
}