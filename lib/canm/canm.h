/*
*
*   Can module
*
*/
#pragma once
#include "vars.h"

#define CANM_API_INIT_U32                  ((uint32_t) 1)

#define CANM_MAX_RELAY_BOARDS_U32          ((uint32_t) 16)   ///< max number of relay boards. Id's 1-16
#define CANM_MAX_PWR_METERS_U32            ((uint32_t) 20)   ///< max number of power meters. Id's 1-20

typedef struct
{
    uint32_t identifier;
    uint8_t data[8];
    uint8_t data_length_code;
} tCANM_MSG;

typedef struct
{
    uint32_t ti_ms_taskDelay_U32;   ///< time in ms between task executions
    tB (*canm_send_callback)(const tCANM_MSG *msg);
    tB (*canm_receive_callback)(tCANM_MSG *msg);
} tCANM_INITDATA_STR;

typedef struct
{
    tCANM_MSG canMsg_str;     // CAN message structure
    tB canRdyForTx_tB;       // message ready for transmission flag
    tB canRdyForParse_tB;    // message ready to be parsed. Only applicable for RX messages
} tCANM_X_CANMSGDATA_STR;

typedef enum
{
    // TX
    CAN_ERROR_MESSAGE0 = 0,
    CAN_ERROR_MESSAGE1,
    CAN_ERROR_MESSAGE2,
    CAN_ERROR_MESSAGE3,
    CAN_DMAS_SEND_MESSAGE0,
    CAN_DMAS_SEND_MESSAGE1,
    CAN_DMAS_SEND_MESSAGE2,
    CAN_DMAS_SEND_MESSAGE3,
    CAN_DMAS_SEND_MESSAGE4,
    CAN_DMAS_SEND_MESSAGE5,
    CAN_DMAS_SEND_MESSAGE6,
    CAN_DMAS_SEND_MESSAGE7,
    CAN_DMAS_SEND_MESSAGE8,
    CAN_DMAS_SEND_MESSAGE9,
    CAN_DMAS_SEND_MESSAGE10,
    CAN_DMAS_SEND_MESSAGE11,
    CAN_DMAS_SEND_MESSAGE12,
    CAN_DMAS_SEND_MESSAGE13,
    CAN_DMAS_SEND_MESSAGE14,
    CAN_DMAS_SEND_MESSAGE15,

    // RX
    CAN_DMAS_COMMAND_MESSAGE,
    CAN_RELAY_INVERT_REQUEST_MESSAGE,
    CAN_DIMMER_EXT_COMMAND_MESSAGE,
    CAN_DIMMER_STATUS_MESSAGE_0,
    CAN_DIMMER_STATUS_MESSAGE_1,

    NUM_OF_CAN_MSG
} tCANM_CANMSGINDX_E;


/**
 * @brief Initialize CAN module
 * @param void
 * @return (void)
 */
void canm_init(tCANM_INITDATA_STR* CanmCfg);

/**
 * @brief Function returns pointer to CAN message data
 * @param msgIndx CAN message index to access
 * @return pointer to CAN message data
 */
tCANM_X_CANMSGDATA_STR* canm_pstr_readCanMsgData(tCANM_CANMSGINDX_E msgIndx);

/**
 * @brief Flag message for transmission
 * @param MsgIndx CAN message to transmit
 * @return (void)
 */
void canm_x_flagCanMsgForTx(tCANM_CANMSGINDX_E MsgIndx);

/**
 * @brief Clear parsing flag for CAN message. Should be done after parsing it
 * @param MsgIndx CAN message to clear parse flag for
 * @return (void)
 */
void canm_x_clearFlagForCanMsgParse(tCANM_CANMSGINDX_E MsgIndx);

/**
 * @brief Transceive function for CAN message tx/rx
 * @param param
 * @return (void)
 */
void canm_transceive_run_5ms(void);