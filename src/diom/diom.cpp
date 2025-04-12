#include "diom.h"
#include "rtdb.h"
#include "tmra.h"
#include "diom_rtdb.h"

static uint32_t diom_nr_moduleId_U32 = 0;
static bool diom_s_moduleInit_tB = false;
static tTMRA_TIMERDATA_STR diom_h_timerHandler_str = tmra_emptyTimerData_str;

static tDIOM_MUXINPUTPIN_E diom_x_muxIn_astr[DIOM_MUX_NUM_OF_INPUTS_U32] =
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
static tDIOM_MUXOUTPUTPIN_E diom_x_muxOut_astr[DIOM_MUX_NUM_OF_OUTPUTS_U32] =
{
    DIOM_MUX_OUT_PIN_0,
    DIOM_MUX_OUT_PIN_1,
};

static tDIOM_GPIODATA_STR diom_x_gpioData_astr[DIOM_NUM_OF_MUX_INPUTS_U32 * DIOM_NUM_OF_MUX_U32];
static tU32 diom_x_gpioStates_U32 = 0; ///< GPIO states as a mask

static void IRAM_ATTR diom_checkPin_isr(tDIOM_GPIODATA_STR* GpioData_pStr, bool NewState_B)
{
    // if cooldown time is 0 then set output to last read state
    if (0 == GpioData_pStr->ti_us_cooldownTime_U32)
    {
        GpioData_pStr->output_B = GpioData_pStr->lastReadState_B;
    }

    // update cooldown timer
    tU32 ti_us_taskTime_U32 = rtdb_read_tU32S(DIOM_TI_TASKTIME_U32);
    GpioData_pStr->ti_us_cooldownTime_U32 -= (GpioData_pStr->ti_us_cooldownTime_U32 > ti_us_taskTime_U32) ? ti_us_taskTime_U32 : GpioData_pStr->ti_us_cooldownTime_U32;

    // if state changed then update output if it's not in cooldown, otherwise refresh cooldown timer
    if (NewState_B != GpioData_pStr->lastReadState_B)
    {
        if (0 == GpioData_pStr->ti_us_cooldownTime_U32)
        {
            GpioData_pStr->output_B = NewState_B;    // update output state
        }
        GpioData_pStr->ti_us_cooldownTime_U32 = rtdb_read_tU32S(DIOM_TI_COOLDOWNPERIOD_U32);    // set cooldown time
        GpioData_pStr->lastReadState_B = NewState_B;    // update last read state
    }
}

static bool IRAM_ATTR diom_run_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    static uint8_t muxSelect_U8 = 0;    ///< Multiplexor select pin combination (0-3)
    // read mux inputs and update output as needed. 0 is "true", as that is when switch is closed
    diom_checkPin_isr(&diom_x_gpioData_astr[ 0 + ((muxSelect_U8 & 0x03) << 1)], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[0]));    // 0,2,4,6
    diom_checkPin_isr(&diom_x_gpioData_astr[ 1 + ((muxSelect_U8 & 0x03) << 1)], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[1]));    // 1,3,5,7
    diom_checkPin_isr(&diom_x_gpioData_astr[ 8 + ((muxSelect_U8 & 0x03) << 1)], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[2]));    // 8,10,12,14
    diom_checkPin_isr(&diom_x_gpioData_astr[ 9 + ((muxSelect_U8 & 0x03) << 1)], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[3]));    // 9,11,13,15
    diom_checkPin_isr(&diom_x_gpioData_astr[16 + ((muxSelect_U8 & 0x03) << 1)], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[4]));    // 16,18,20,22
    diom_checkPin_isr(&diom_x_gpioData_astr[17 + ((muxSelect_U8 & 0x03) << 1)], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[5]));    // 17,19,21,23
    diom_checkPin_isr(&diom_x_gpioData_astr[24 + ((muxSelect_U8 & 0x03) << 1)], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[6]));    // 24,26,28,30
    diom_checkPin_isr(&diom_x_gpioData_astr[25 + ((muxSelect_U8 & 0x03) << 1)], !pina_getGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxIn_astr[7]));    // 25,27,29,31

    // update GPIO status mask
    tU32 tempGpioStatusMask_U32 = 0;
    for (uint32_t i_U32 = 0; i_U32 < (DIOM_NUM_OF_MUX_INPUTS_U32 * DIOM_NUM_OF_MUX_U32); i_U32++)
    {
        tempGpioStatusMask_U32 |= diom_x_gpioData_astr[i_U32].output_B << i_U32;
    }
    diom_x_gpioStates_U32 = tempGpioStatusMask_U32;
    rtdb_write_tU32S(DIOM_TI_INPUTSTATES_U32, diom_x_gpioStates_U32);    // update RTDB with new GPIO states

    // update mux selector and restart timer
    muxSelect_U8++;
    pina_setGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxOut_astr[0], (muxSelect_U8 >> 0) & 0x01);
    pina_setGpioLevel((PINA_nr_GPIO_NUM_E) diom_x_muxOut_astr[1], (muxSelect_U8 >> 1) & 0x01);
    diom_h_timerHandler_str.alarmConfig_str.alarm_count = rtdb_read_tU32S(DIOM_TI_TASKTIME_U32);
    tmra_startTimer(&diom_h_timerHandler_str); // restart timer
    return 1;    // return 1 to keep the timer running
}

void diom_init(tDIOM_INITDATA_STR* DiomCfg)
{
    if (true == diom_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, diom_nr_moduleId_U32, 0, DIOM_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == DiomCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, diom_nr_moduleId_U32, 0, DIOM_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        diom_nr_moduleId_U32 = DiomCfg->nr_moduleId_U32;
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

        diom_rtdb_init();    // initialize RTDB

        // start timer for the task
        static gptimer_config_t timer_config =
        {
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 1000000, // 1MHz, 1 tick=1us
            .intr_priority = 0,
            .flags = {
                .intr_shared = 0,
                .backup_before_sleep = 0,
            }
        };
        static gptimer_event_callbacks_t cbsInitial =
        {
            .on_alarm = diom_run_isr,
        };
        diom_h_timerHandler_str.cbs = cbsInitial;
        diom_h_timerHandler_str.timerConfig_str = timer_config;
        diom_h_timerHandler_str.alarmConfig_str.alarm_count = rtdb_read_tU32S(DIOM_TI_TASKTIME_U32);
        tmra_createTimer(&diom_h_timerHandler_str);
        tmra_startTimer(&diom_h_timerHandler_str);
    }
}
