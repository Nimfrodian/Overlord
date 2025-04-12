/*
*
*   timer abstraction file
*
*/
#pragma once

#include "driver/uart.h"
#include "driver/gptimer.h"
#include "errh.h"

#define TMRA_API_INIT_U32                  ((uint32_t) 1)
#define TMRA_API_CREATE_TIMER_U32          ((uint32_t) 2)
#define TMRA_API_START_TIMER_U32           ((uint32_t) 3)
#define TMRA_API_STOP_TIMER_U32            ((uint32_t) 4)

#define TMRA_ERR_MALLOC_RETURNED_NULL_U32  ((uint32_t) 1)
#define TMRA_ERR_CANNOT_CREATE_TIMER_U32   ((uint32_t) 2)
#define TMRA_ERR_CANNOT_ADD_CB_TIMER_U32   ((uint32_t) 3)
#define TMRA_ERR_CANNOT_ENABLE_TIMER_U32   ((uint32_t) 4)
#define TMRA_ERR_CANNOT_SET_ALARM_U32      ((uint32_t) 5)
#define TMRA_ERR_CANNOT_START_TIMER_U32    ((uint32_t) 6)
#define TMRA_ERR_CANNOT_DISABLE_TIMER_U32  ((uint32_t) 7)
#define TMRA_ERR_CANNOT_STOP_TIMER_U32     ((uint32_t) 8)


typedef struct
{
    uint32_t nr_moduleId_U32;       ///< ID of the module
} tTMRA_INITDATA_STR;

typedef gptimer_handle_t tTMRA_TIMERHANDLE_STR;

typedef struct
{
    gptimer_handle_t timerHandle_str;
    gptimer_alarm_config_t alarmConfig_str;
    gptimer_config_t timerConfig_str;
    gptimer_event_callbacks_t cbs;
} tTMRA_TIMERDATA_STR;

extern tTMRA_TIMERDATA_STR tmra_emptyTimerData_str;

/**
 * @brief Initialized timer abstraction module (TMRA)
 * @param TmraCfg pointer to configuration varible
 * @return (void)
 */
void tmra_init(tTMRA_INITDATA_STR* TmraCfg);

/**
 * @brief Functions returns the time in microseconds since program start
 * @param void
 * @return time in microseconds since program start
 */
int64_t tmra_ti_us_getCurrentTime_S64(void);

/**
 * @brief Function starts a timer with a given delay. Once the time is up the function is executed
 * @param TimerHandlePtr Pointer to a timer handle
 * @return error code
 */
uint32_t tmra_startTimer(tTMRA_TIMERDATA_STR* TimerHandle);

/**
 * @brief Function stops a timer
 * @param TimerHandlePtr pointer to a timer handle
 * @return error code
 */
uint32_t tmra_stopTimer(tTMRA_TIMERDATA_STR* TmraTimerHandle);

/**
 * @brief Function setups a timer
 * @param TimerHandlePtr pointer to a timer handle
 * @param timerFunc function to be executed
 * @return error code
 */
uint32_t tmra_createTimer(tTMRA_TIMERDATA_STR* TmraTimerHandle);