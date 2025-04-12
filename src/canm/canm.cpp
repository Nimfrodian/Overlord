#include "canm.h"

static bool canm_s_moduleInit_tB = false;
static uint32_t canm_nr_moduleId_U32 = 0;

const twai_general_config_t canm_x_genConfig_str =
{
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

tCANM_X_CANMSGDATA_STR canm_x_canMsgs_astr[NUM_OF_CAN_MSG];

void canm_init(tCANM_INITDATA_STR* CanmCfg)
{
    if (true == canm_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, canm_nr_moduleId_U32, 0, CANM_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == CanmCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, canm_nr_moduleId_U32, 0, CANM_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        for (uint16_t canMsgIndx_U16 = 0; canMsgIndx_U16 < NUM_OF_CAN_MSG; canMsgIndx_U16++)
        {
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.extd = 0,
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.rtr = 0,
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.ss = 0,
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.self = 0,
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.dlc_non_comp = 0,
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.reserved = 0,
            canm_x_canMsgs_astr[canMsgIndx_U16].canRdyForTx_tB = 0;                // set transmit flag to 0 on init
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.data_length_code = 8;    // all messages are DLC 8
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.data[0] = 0;             // clear the data location
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.data[1] = 0;
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.data[2] = 0;
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.data[3] = 0;
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.data[4] = 0;
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.data[5] = 0;
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.data[6] = 0;
            canm_x_canMsgs_astr[canMsgIndx_U16].canMsg_str.data[7] = 0;
        }

        // initialize CAN Module
        twai_driver_install(&canm_x_genConfig_str, &canm_x_timingConfig_str, &canm_x_filter_str);

        twai_start();

        canm_nr_moduleId_U32 = CanmCfg->nr_moduleId_U32;
        canm_s_moduleInit_tB = true;    // only init once
    }
}

static void canm_saveMsg(tCANM_CANMSGINDX_E MsgIndx_E, twai_message_t* SourceMsg_pstr)
{
    // get CAN message pointer to where the data should be saved
    tCANM_X_CANMSGDATA_STR* destMsg_pstr = canm_pstr_readCanMsgData(MsgIndx_E);

    // copy the data from source message to destination message
    for (uint8_t i = 0; i < SourceMsg_pstr->data_length_code; i++)
    {
        destMsg_pstr->canMsg_str.data[i] = SourceMsg_pstr->data[i];
    }

    // save message ID
    destMsg_pstr->canMsg_str.identifier = SourceMsg_pstr->identifier;

    // flag that message is ready to be parsed
    destMsg_pstr->canRdyForParse_tB = 1;
}

void canm_transceive_run_5ms(void)
{
    // transmit
    {
        // check if any messages are waiting to be sent
        for (uint16_t msgIndx_U16 = 0; msgIndx_U16 < NUM_OF_CAN_MSG; msgIndx_U16++)
        {
            // get message data pointer
            tCANM_X_CANMSGDATA_STR* canMsgPtr = canm_pstr_readCanMsgData((tCANM_CANMSGINDX_E) msgIndx_U16);

            // check if message needs to be sent
            if (1 == canMsgPtr->canRdyForTx_tB)
            {
                // try to copy the message into hardware transmit buffer
                if (ESP_OK == twai_transmit(&canMsgPtr->canMsg_str, 0))
                {
                    // clear "send" flag if copy was successful
                    canMsgPtr->canRdyForTx_tB = false;
                    break;
                }
            }
        }
    }

    // receive
    {
        twai_message_t rxMessage;
        if (ESP_OK == twai_receive(&rxMessage, 0))
        {
            uint32_t rxId = rxMessage.identifier;
            switch (rxId)
            {
                case 0x95:  // message CAN_COMMAND_MESSAGE
                {
                    canm_saveMsg(CAN_COMMAND_MESSAGE, &rxMessage);
                    break;
                }
                // message CAN_DMAS_COMMAND_MESSAGE
                // fallthrough
                case (0x10):    // DMAS_TU8
                case (0x11):    // DMAS_TU16
                case (0x12):    // DMAS_TU32
                case (0x13):    // DMAS_TS8
                case (0x14):    // DMAS_TS16
                case (0x15):    // DMAS_TS32
                case (0x16):    // DMAS_TF32S
                case (0x17):    // DMAS_TBS
                case (0x18):    // DMAS_TES
                {
                    canm_saveMsg(CAN_DMAS_COMMAND_MESSAGE, &rxMessage);
                    break;
                }

                default:
                break;
            }
        }
    }
}

tCANM_X_CANMSGDATA_STR* canm_pstr_readCanMsgData(tCANM_CANMSGINDX_E MsgIndx)
{
    uint16_t rMsgIndx_U16 = 0;
    if (MsgIndx < NUM_OF_CAN_MSG)
    {
        rMsgIndx_U16 = MsgIndx;
    }
    return &canm_x_canMsgs_astr[rMsgIndx_U16];
}
/**
 * @brief Function flags the CAN message for transmission so that it will be sent
 * @param MsgIndx message that should be sent
 * @return (void)
 */
void canm_x_flagCanMsgForTx(tCANM_CANMSGINDX_E MsgIndx)
{
    tCANM_X_CANMSGDATA_STR* msgPtr_pstr = canm_pstr_readCanMsgData(MsgIndx);
    msgPtr_pstr->canRdyForTx_tB = true;
}

/**
 * @brief Function clears the flag to CAN message parsing
 * @param MsgIndx message index for which the parsing flag should be cleared
 * @return (void)
 */
void canm_x_clearFlagForCanMsgParse(tCANM_CANMSGINDX_E MsgIndx)
{
    tCANM_X_CANMSGDATA_STR* msgPtr_pstr = canm_pstr_readCanMsgData(MsgIndx);
    msgPtr_pstr->canRdyForParse_tB = false;
}