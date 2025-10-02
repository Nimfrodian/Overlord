#include "diom.h"
#include "rtdb.h"
#include "tmra.h"
#include "mdll.h"
#include "rtdb.h"

static tB diom_s_moduleInit_tB = false;
static tTMRA_TIMERDATA_STR diom_h_timerHandler_str = tmra_emptyTimerData_str;

static tU32 diom_ti_us_task_time_U32 = rtdb_read_tU32S(RTDB_DIOM_TI_US_TASKTIME_U32);
static tU32 diom_ti_us_cooldown_U32 = rtdb_read_tU32S(RTDB_DIOM_TI_US_COOLDOWNPERIOD_U32);

static const tDIOM_MUXINPUTPIN_E diom_x_muxIn_astr[DIOM_MUX_NUM_OF_INPUTS_U32] =
{
    DIOM_MUX_IN_PIN_0,
    DIOM_MUX_IN_PIN_1,
    DIOM_MUX_IN_PIN_2,
    DIOM_MUX_IN_PIN_3,
    DIOM_MUX_IN_PIN_4,
    DIOM_MUX_IN_PIN_5,
    DIOM_MUX_IN_PIN_6,
    DIOM_MUX_IN_PIN_7,
};
static const tDIOM_MUXOUTPUTPIN_E diom_x_muxOut_astr[DIOM_MUX_NUM_OF_OUTPUTS_U32] =
{
    DIOM_MUX_OUT_PIN_0,
    DIOM_MUX_OUT_PIN_1,
};

static const tU32 diom_x_mapping_U32[DIOM_NUM_OF_MUX_INPUTS_U32 * DIOM_NUM_OF_MUX_U32] =
{
    2,   1,  0,  3,      7,  4,  6,  5,
    15, 12, 14, 13,     10,  9,  8, 11,
    31, 28, 30, 29,     26, 25, 24, 27,
    23, 20, 22, 21,     18, 17, 16, 19
};
static tDIOM_GPIODATA_STR diom_x_gpioData_astr[DIOM_NUM_OF_MUX_INPUTS_U32 * DIOM_NUM_OF_MUX_U32] = {};
static tU32 diom_x_gpioStates_U32 = 0; ///< GPIO states as a mask

static void IRAM_ATTR diom_updateCooldownTimer_isr(tDIOM_GPIODATA_STR* GpioData_pStr)
{
    // if cooldown time is 0 then set output to last read state
    if (0 == GpioData_pStr->ti_us_cooldownTime_U32)
    {
        GpioData_pStr->output_B = GpioData_pStr->lastReadState_B;
    }

    // update cooldown timer
    tU32 ti_us_taskTime_U32 = diom_ti_us_task_time_U32;
    GpioData_pStr->ti_us_cooldownTime_U32 -= (GpioData_pStr->ti_us_cooldownTime_U32 > ti_us_taskTime_U32) ? ti_us_taskTime_U32 : GpioData_pStr->ti_us_cooldownTime_U32;
}

static void IRAM_ATTR diom_checkPin_isr(tDIOM_GPIODATA_STR* GpioData_pStr, tB NewState_B)
{
    // if state changed then update output if it's not in cooldown, otherwise refresh cooldown timer
    if (NewState_B != GpioData_pStr->lastReadState_B)
    {
        if (0 == GpioData_pStr->ti_us_cooldownTime_U32)
        {
            GpioData_pStr->output_B = NewState_B;    // update output state
        }
        GpioData_pStr->ti_us_cooldownTime_U32 = diom_ti_us_cooldown_U32;    // set cooldown time
        GpioData_pStr->lastReadState_B = NewState_B;    // update last read state
    }
}

static bool IRAM_ATTR diom_run_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    static uint8_t muxSelect_U8 = 0;    ///< Multiplexor select pin combination (0-3)

    // update timer for all GPIOs
    for (uint32_t i_U32 = 0; i_U32 < (DIOM_NUM_OF_MUX_INPUTS_U32 * DIOM_NUM_OF_MUX_U32); i_U32++)
    {
        diom_updateCooldownTimer_isr(&diom_x_gpioData_astr[i_U32]);
    }

    // compensate for pin routing on PCB
    tU8 mapped_0 = diom_x_mapping_U32[ 0 + (muxSelect_U8 & 0x03)];
    tU8 mapped_1 = diom_x_mapping_U32[ 4 + (muxSelect_U8 & 0x03)];
    tU8 mapped_2 = diom_x_mapping_U32[ 8 + (muxSelect_U8 & 0x03)];
    tU8 mapped_3 = diom_x_mapping_U32[12 + (muxSelect_U8 & 0x03)];
    tU8 mapped_4 = diom_x_mapping_U32[16 + (muxSelect_U8 & 0x03)];
    tU8 mapped_5 = diom_x_mapping_U32[20 + (muxSelect_U8 & 0x03)];
    tU8 mapped_6 = diom_x_mapping_U32[24 + (muxSelect_U8 & 0x03)];
    tU8 mapped_7 = diom_x_mapping_U32[28 + (muxSelect_U8 & 0x03)];
    // read mux inputs and update output as needed. 0 is "true", as that is when switch is closed
    diom_checkPin_isr(&diom_x_gpioData_astr[mapped_0], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[0]));
    diom_checkPin_isr(&diom_x_gpioData_astr[mapped_1], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[1]));
    diom_checkPin_isr(&diom_x_gpioData_astr[mapped_2], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[2]));
    diom_checkPin_isr(&diom_x_gpioData_astr[mapped_3], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[3]));
    diom_checkPin_isr(&diom_x_gpioData_astr[mapped_4], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[4]));
    diom_checkPin_isr(&diom_x_gpioData_astr[mapped_5], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[5]));
    diom_checkPin_isr(&diom_x_gpioData_astr[mapped_6], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[6]));
    diom_checkPin_isr(&diom_x_gpioData_astr[mapped_7], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[7]));

    // update GPIO status mask
    tU32 tempGpioStatusMask_U32 = 0;
    for (uint32_t i_U32 = 0; i_U32 < (DIOM_NUM_OF_MUX_INPUTS_U32 * DIOM_NUM_OF_MUX_U32); i_U32++)
    {
        tempGpioStatusMask_U32 |= diom_x_gpioData_astr[i_U32].output_B << i_U32;
    }
    diom_x_gpioStates_U32 = tempGpioStatusMask_U32;

    // update mux selector and restart timer
    muxSelect_U8++;
    pina_setGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxOut_astr[0], (muxSelect_U8 >> 0) & 0x01);
    pina_setGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxOut_astr[1], (muxSelect_U8 >> 1) & 0x01);
    diom_h_timerHandler_str.alarmConfig_str.alarm_count = diom_ti_us_cooldown_U32;
    tmra_startTimer(&diom_h_timerHandler_str); // restart timer
    return 1;    // return 1 to keep the timer running
}

void diom_init(tDIOM_INITDATA_STR* DiomCfg)
{
    if (true == diom_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, MODULE_DIOM, 0, DIOM_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == DiomCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, MODULE_DIOM, 0, DIOM_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        diom_s_moduleInit_tB = true;    // only init once

        // set gpio pins as inputs/outputs
        for (uint32_t i_U32 = 0; i_U32 < DIOM_MUX_NUM_OF_INPUTS_U32; i_U32++)
        {
            pina_setGpioAsInput((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[i_U32]);
        }
        for (uint32_t i_U32 = 0; i_U32 < DIOM_MUX_NUM_OF_OUTPUTS_U32; i_U32++)
        {
            pina_setGpioAsOutput((PINA_nr_GPIO_NUM_E) diom_x_muxOut_astr[i_U32]);
        }

        // start timer for the task
        static gptimer_config_t timer_config =
        {
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 1000000, // 1MHz, 1 tick=1us
            .intr_priority = 0,
            .flags = {
                .intr_shared = 0,
                .allow_pd = 0,
                .backup_before_sleep = 0,
            }
        };
        static gptimer_event_callbacks_t cbsInitial =
        {
            .on_alarm = diom_run_isr,
        };
        diom_h_timerHandler_str.cbs = cbsInitial;
        diom_h_timerHandler_str.timerConfig_str = timer_config;
        diom_h_timerHandler_str.alarmConfig_str.alarm_count = rtdb_read_tU32S(RTDB_DIOM_TI_US_TASKTIME_U32);
        tmra_createTimer(&diom_h_timerHandler_str);
        tmra_startTimer(&diom_h_timerHandler_str);
    }
}

void diom_run_5ms(void)
{
    diom_ti_us_cooldown_U32 = rtdb_read_tU32S(RTDB_DIOM_TI_US_COOLDOWNPERIOD_U32);
    diom_ti_us_task_time_U32 = rtdb_read_tU32S(RTDB_DIOM_TI_US_TASKTIME_U32);
    rtdb_write_tU32S(RTDB_DIOM_X_INPUTSTATES_U32, diom_x_gpioStates_U32);    // update RTDB with new GPIO states
}