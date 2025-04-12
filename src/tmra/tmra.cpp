#include "tmra.h"
#include "rtdb_vars.h"

static bool tmra_s_moduleInit_tB = false;
static uint32_t tmra_nr_moduleId_U32 = 0;
static tTMRA_TIMERHANDLE_STR tmra_x_internalTimer_strc;

static const gptimer_config_t tmra_x_internalTimerConfig_strc = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
        .intr_priority = 0,
        .flags = {
            .intr_shared = 0,
            .backup_before_sleep = 0,
        }
    };

tTMRA_TIMERDATA_STR tmra_emptyTimerData_str =
{
    .timerHandle_str = nullptr,
    .alarmConfig_str = {
        .alarm_count = 0,
        .reload_count = 0,
        .flags = {
            .auto_reload_on_alarm = 0,
        }
    },
    .timerConfig_str = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
        .intr_priority = 0,
        .flags = {
            .intr_shared = 0,
            .backup_before_sleep = 0,
        }
    },
    .cbs = {
        .on_alarm = nullptr,
    }
};

void tmra_init(tTMRA_INITDATA_STR* TmraCfg)
{
    if (true == tmra_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, tmra_nr_moduleId_U32, 0, TMRA_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == TmraCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, tmra_nr_moduleId_U32, 0, TMRA_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        gptimer_new_timer(&tmra_x_internalTimerConfig_strc, &tmra_x_internalTimer_strc);
        gptimer_enable(tmra_x_internalTimer_strc);
        gptimer_start(tmra_x_internalTimer_strc);
        tmra_nr_moduleId_U32 = TmraCfg->nr_moduleId_U32;
        tmra_s_moduleInit_tB = true;
    }
}

int64_t tmra_ti_us_getCurrentTime_S64(void)
{
    uint64_t tmra_ti_us_rawSysTi_U64 = 0;
    if (true == tmra_s_moduleInit_tB)
    {
        gptimer_get_raw_count(tmra_x_internalTimer_strc, &tmra_ti_us_rawSysTi_U64);
    }
    return (int64_t) tmra_ti_us_rawSysTi_U64;
}

uint32_t tmra_stopTimer(tTMRA_TIMERDATA_STR* TmraTimerHandle)
{
    uint32_t err = 0;
    uint64_t tmra_ti_us_rawSysTi_U64 = 0;
    gptimer_handle_t* TimerHandle = &TmraTimerHandle->timerHandle_str;

    gptimer_get_raw_count(*TimerHandle, &tmra_ti_us_rawSysTi_U64);

    if (0 < tmra_ti_us_rawSysTi_U64)
    {
        err = gptimer_stop(TmraTimerHandle->timerHandle_str);
        if (err) {errh_reportError(ERRH_NOTIF, tmra_nr_moduleId_U32, 0, TMRA_API_STOP_TIMER_U32, TMRA_ERR_CANNOT_STOP_TIMER_U32);};
        gptimer_set_raw_count(*TimerHandle, 0);
        err = gptimer_disable(TmraTimerHandle->timerHandle_str);
        if (err) {errh_reportError(ERRH_NOTIF, tmra_nr_moduleId_U32, 0, TMRA_API_STOP_TIMER_U32, TMRA_ERR_CANNOT_DISABLE_TIMER_U32);};
    }
    return err;
}

uint32_t tmra_startTimer(tTMRA_TIMERDATA_STR* TmraTimerHandle)
{
    uint32_t err = 0;

    if (nullptr == TmraTimerHandle)
    {
        err = 1;
        errh_reportError(ERRH_ERROR_CRITICAL, tmra_nr_moduleId_U32, 0, TMRA_API_START_TIMER_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        gptimer_handle_t* TimerHandle = &TmraTimerHandle->timerHandle_str;
        gptimer_alarm_config_t* alarm_config = &TmraTimerHandle->alarmConfig_str;

        tmra_stopTimer(TmraTimerHandle);

        err = gptimer_enable(*TimerHandle);
        if (err) {errh_reportError(ERRH_ERROR_CRITICAL, tmra_nr_moduleId_U32, 0, TMRA_API_START_TIMER_U32, TMRA_ERR_CANNOT_ENABLE_TIMER_U32);};

        err = gptimer_set_alarm_action(*TimerHandle, alarm_config);
        if (err) {errh_reportError(ERRH_NOTIF, tmra_nr_moduleId_U32, 0, TMRA_API_START_TIMER_U32, TMRA_ERR_CANNOT_SET_ALARM_U32);};

        err = gptimer_start(*TimerHandle);
        if (err) {errh_reportError(ERRH_NOTIF, tmra_nr_moduleId_U32, 0, TMRA_API_START_TIMER_U32, TMRA_ERR_CANNOT_START_TIMER_U32);};
    }
    return err;
}

uint32_t tmra_createTimer(tTMRA_TIMERDATA_STR* TmraTimerHandle)
{
    uint32_t err = 0;
    err = gptimer_new_timer(&TmraTimerHandle->timerConfig_str, &TmraTimerHandle->timerHandle_str);
    if (err)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, tmra_nr_moduleId_U32, err, TMRA_API_CREATE_TIMER_U32, TMRA_ERR_CANNOT_CREATE_TIMER_U32);
    }

    err = gptimer_register_event_callbacks(TmraTimerHandle->timerHandle_str, &TmraTimerHandle->cbs, nullptr);
    if (err)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, tmra_nr_moduleId_U32, err, TMRA_API_CREATE_TIMER_U32, TMRA_ERR_CANNOT_ADD_CB_TIMER_U32);
    }

    return err;
}