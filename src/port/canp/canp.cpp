#include "canp.h"
#include "canm.h"

#if defined(PLATFORM_ESP32)

#include "driver/twai.h"

const twai_general_config_t canm_x_genConfig_str =
{
    .controller_id = 0,
    .mode = TWAI_MODE_NORMAL,
    .tx_io = GPIO_NUM_5,
    .rx_io = GPIO_NUM_4,
    .clkout_io = (gpio_num_t) -1,
    .bus_off_io = (gpio_num_t) -1,
    .tx_queue_len = 16,
    .rx_queue_len = 16,
    .alerts_enabled = 0,
    .clkout_divider = 0,
    .intr_flags = 0,
};

const twai_timing_config_t canm_x_timingConfig_str = TWAI_TIMING_CONFIG_250KBITS();

const twai_filter_config_t canm_x_filter_str =
{
    .acceptance_code = 0,
    .acceptance_mask = 0xFFFFFFFF,
    .single_filter = true,
};

tB canp_init(tCANP_INITDATA_STR* CanpCfg)
{
    tB toReturn = NO_ERROR;

    tCANM_INITDATA_STR CanmCfg =
    {
        .ti_ms_taskDelay_U32 = CanpCfg->ti_ms_taskDelay_U32, // executed in main task
        .canm_send_callback = canp_send,
        .canm_receive_callback = canp_receive,
    };
    canm_init(&CanmCfg);

    twai_driver_install(&canm_x_genConfig_str, &canm_x_timingConfig_str, &canm_x_filter_str);
    if (ESP_OK != twai_start())
    {
        toReturn = ERROR;
    }
    return toReturn;
}

tB canp_send(const tCANM_MSG *msg)
{
    twai_message_t hwMsg = {0};
    hwMsg.identifier = msg->identifier;
    hwMsg.data_length_code = msg->data_length_code;
    hwMsg.data[0] = msg->data[0];
    hwMsg.data[1] = msg->data[1];
    hwMsg.data[2] = msg->data[2];
    hwMsg.data[3] = msg->data[3];
    hwMsg.data[4] = msg->data[4];
    hwMsg.data[5] = msg->data[5];
    hwMsg.data[6] = msg->data[6];
    hwMsg.data[7] = msg->data[7];
    hwMsg.extd = 0;
    hwMsg.rtr = 0;
    hwMsg.ss = 0;
    hwMsg.self = 0;
    hwMsg.dlc_non_comp = 0;
    hwMsg.reserved = 0;

    esp_err_t res = twai_transmit(&hwMsg, pdMS_TO_TICKS(100));
    return (res == ESP_OK);
}

tB canp_receive(tCANM_MSG *msg)
{
    tB toReturn = NO_ERROR;
    twai_message_t hwMsg = {0};
    if (ESP_OK == twai_receive(&hwMsg, 0))
    {
        msg->identifier = hwMsg.identifier;
        msg->data_length_code = hwMsg.data_length_code;
        msg->data[0] = hwMsg.data[0];
        msg->data[1] = hwMsg.data[1];
        msg->data[2] = hwMsg.data[2];
        msg->data[3] = hwMsg.data[3];
        msg->data[4] = hwMsg.data[4];
        msg->data[5] = hwMsg.data[5];
        msg->data[6] = hwMsg.data[6];
        msg->data[7] = hwMsg.data[7];
    }
    else {
        toReturn = ERROR;
    }
    return toReturn;
}
#elif defined(PLATFORM_LOCALHOST)
    tB canp_init(tCANP_INITDATA_STR* CanpCfg)
    {
        return NO_ERROR;
    }
    tB canp_send(const tCANM_MSG *msg)
    {
        return NO_ERROR;
    }
    tB canp_receive(tCANM_MSG *msg)
    {
        return NO_ERROR;
    }
#endif