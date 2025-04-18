#include "canm.h"
#include "canm_rtdb.h"
#include "mdll.h"

static bool canm_s_moduleInit_tB = false;
static uint32_t canm_ti_ms_taskDelay_U32 = 0;

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

tCANM_X_CANMSGDATA_STR canm_x_canMsgs_astr[NUM_OF_CAN_MSG];

void canm_init(tCANM_INITDATA_STR* CanmCfg)
{
    if (true == canm_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, MODULE_CANM, 0, CANM_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == CanmCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, MODULE_CANM, 0, CANM_API_INIT_U32, ERRH_POINTER_IS_NULL);
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

        canm_ti_ms_taskDelay_U32 = CanmCfg->ti_ms_taskDelay_U32;

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
        // TODO: update
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
    // (new) transmit
    {
        static uint32_t cntr_10ms = 0;   // counter for  10ms transmit
        static uint32_t cntr_100ms = 0;  // counter for 100ms transmit
        static uint32_t cntr_1s = 0;     // counter for    1s transmit
        {
            cntr_10ms += canm_ti_ms_taskDelay_U32;
            cntr_100ms += canm_ti_ms_taskDelay_U32;
            cntr_1s += canm_ti_ms_taskDelay_U32;

            if (10 <= cntr_10ms)
            {
                cntr_10ms = 0;
            }
            if (100 <= cntr_100ms)
            {
                // send 0x150 GPIO feedback status
                {
                    static twai_message_t canMsg_0x150 = {};
                    canMsg_0x150.identifier = 0x150;
                    canMsg_0x150.data_length_code = 8;
                    for (tU8 byteIndx_U8 = 0; byteIndx_U8 < 4; byteIndx_U8++)
                    {
                        canMsg_0x150.data[byteIndx_U8] = 0;    // clear the data location
                        for (tU8 bitIndx_U8 = 0; bitIndx_U8 < 8; bitIndx_U8++)
                        {
                            tU8 index_U8 = (RTDB_CANM_S_TXGPIOSTATES_AB_0 + (byteIndx_U8 * 8) + bitIndx_U8);
                            rtdb_write_tBS((tBSEnumT) index_U8, rtdb_read_tU32S(RTDB_DIOM_X_INPUTSTATES_U32) >> (byteIndx_U8*8 + bitIndx_U8) & 0x01);    // copy from GPIO state
                            canMsg_0x150.data[byteIndx_U8] |= (rtdb_read_tBS((tBSEnumT) index_U8) << bitIndx_U8);
                        }
                    }
                    twai_transmit(&canMsg_0x150, 0);
                }
                // send 0x110 Relay feedback status
                {
                    static twai_message_t canMsg_0x110 = {};
                    canMsg_0x110.identifier = 0x110;
                    canMsg_0x110.data_length_code = 8;
                    for (tU8 byteIndx_U8 = 0; byteIndx_U8 < 8; byteIndx_U8++)
                    {
                        canMsg_0x110.data[byteIndx_U8] = 0;    // clear the data location
                        for (tU8 bitIndx_U8 = 0; bitIndx_U8 < 8; bitIndx_U8++)
                        {
                            tU32 index_U32 = (RTDB_CANM_S_TXRELAYSTATES_AB_0 + (byteIndx_U8 * 8) + bitIndx_U8);
                            rtdb_write_tBS((tBSEnumT) index_U32, (rtdb_read_tU8S((tU8SEnumT)(RTDB_MBCM_X_ACTUALRELAYSTATES_AU8_0 + byteIndx_U8)) >> bitIndx_U8) & 0x01);    // copy from Relay state
                            canMsg_0x110.data[byteIndx_U8] |= (rtdb_read_tBS((tBSEnumT) index_U32) << bitIndx_U8);
                        }
                    }
                    twai_transmit(&canMsg_0x110, 0);
                }
                // send 0x111 Relay feedback status
                {
                    static twai_message_t canMsg_0x111 = {};
                    canMsg_0x111.identifier = 0x111;
                    canMsg_0x111.data_length_code = 8;
                    for (tU8 byteIndx_U8 = 0; byteIndx_U8 < 8; byteIndx_U8++)
                    {
                        canMsg_0x111.data[byteIndx_U8] = 0;    // clear the data location
                        for (tU8 bitIndx_U8 = 0; bitIndx_U8 < 8; bitIndx_U8++)
                        {
                            tU32 index_U32 = (RTDB_CANM_S_TXRELAYSTATES_AB_64 + (byteIndx_U8 * 8) + bitIndx_U8);
                            rtdb_write_tBS((tBSEnumT) index_U32, (rtdb_read_tU8S((tU8SEnumT)(RTDB_MBCM_X_ACTUALRELAYSTATES_AU8_8 + byteIndx_U8)) >> bitIndx_U8) & 0x01);    // copy from Relay state
                            canMsg_0x111.data[byteIndx_U8] |= (rtdb_read_tBS((tBSEnumT) index_U32) << bitIndx_U8);
                        }
                    }
                    twai_transmit(&canMsg_0x111, 0);
                }
                cntr_100ms = 0;
            }
            if (cntr_1s > 1000)
            {
                // send 0x1F5 + id + varIndx message for power meter, one power meter's info per second
                {
                    twai_message_t canMsg_sdm120m[11] = {}; // 11 messages per power module
                    static tU8 moduleIndx = 0;  // up to CANM_MAX_PWR_METERS_U32
                    for (tU8 i = 0; i < 11; i++)
                    {
                        canMsg_sdm120m[i].identifier = 0x1F5 + i + moduleIndx * 11;
                        canMsg_sdm120m[i].data_length_code = 8;

                        tU32 data_0 = 0;    //  0 - 31 bits
                        tU32 data_1 = 0;    // 32 - 63 bits
                        tF32 data_0_f32 = 0.0f;
                        tF32 data_1_f32 = 0.0f;
                        // TODO: if (SIGNAL_OK == rtdb_getSS(...))
                        data_0_f32 = rtdb_read_tF32S((tF32SEnumT)(RTDB_MBCM_X_SDM120MREADINGS_0_AF32_0 + (i * 2) + 0 + moduleIndx * 21)); // 0,2,4,6,8,10,12,14,16,18,20   21,23,25,27,29,31,33,35,37,39,41 ...
                        data_0 = *(tU32*)((void*) &data_0_f32);
                        if (i < 10) // Skip last one as that would be index 21 - out of bounds
                        {
                            data_1_f32 = rtdb_read_tF32S((tF32SEnumT)(RTDB_MBCM_X_SDM120MREADINGS_0_AF32_0 + (i * 2) + 1 + moduleIndx * 21)); // 1,3,5,7,9,11,13,15,17,19   22,24,26,28,30,32,34,36,38,40 ...
                            data_1 = *(tU32*)((void*) &data_1_f32);
                        }
                        data_1 = *(tU32*)((void*) &data_1_f32);
                        canMsg_sdm120m[i].data[0] = (data_0 >> 0) & 0xFF;
                        canMsg_sdm120m[i].data[1] = (data_0 >> 8) & 0xFF;
                        canMsg_sdm120m[i].data[2] = (data_0 >> 16) & 0xFF;
                        canMsg_sdm120m[i].data[3] = (data_0 >> 24) & 0xFF;

                        canMsg_sdm120m[i].data[4] = (data_1 >> 0) & 0xFF;
                        canMsg_sdm120m[i].data[5] = (data_1 >> 8) & 0xFF;
                        canMsg_sdm120m[i].data[6] = (data_1 >> 16) & 0xFF;
                        canMsg_sdm120m[i].data[7] = (data_1 >> 24) & 0xFF;
                        twai_transmit(&canMsg_sdm120m[i], 0);
                    }
                    moduleIndx++;
                    if (moduleIndx >= CANM_MAX_PWR_METERS_U32)
                    {
                        moduleIndx = 0;
                    }
                }
                cntr_1s = 0;
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
                // message CAN_RELAY_INVERT_REQUEST_MESSAGE
                case (0x100):
                {
                    // old: canm_saveMsg(CAN_RELAY_INVERT_REQUEST_MESSAGE, &rxMessage);
                    for (tU8 i = 0; i < 64; i++)
                    {
                        tB relayInvertReq_B = (rxMessage.data[i / 8] >> (i % 8)) & 0x01;
                        rtdb_write_tBS((tBSEnumT)(RTDB_CANM_S_RXRELAYINVERTREQ_AB_0 + i), relayInvertReq_B);
                    }
                    break;
                }
                case (0x101):
                {
                    for (tU8 i = 0; i < 64; i++)
                    {
                        tB relayInvertReq_B = (rxMessage.data[i / 8] >> (i % 8)) & 0x01;
                        rtdb_write_tBS((tBSEnumT)(RTDB_CANM_S_RXRELAYINVERTREQ_AB_0 + i + 64), relayInvertReq_B);
                    }
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