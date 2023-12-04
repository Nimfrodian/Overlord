#ifndef RELAY_MODULE_H
#define RELAY_MODULE_H

#include "freertos/FreeRTOS.h"  // for basic data types

const uint8_t NUM_OF_RELAYS_PER_WAVESHARE_BOARD = 8;

const uint8_t RELAY_MODULE_MB0_TRANSMIT_MSG_SIZE = 10;

enum
{
    RELAY_MODULE_BOARD_INDEX_0 = 0,
    RELAY_MODULE_BOARD_INDEX_1,
    RELAY_MODULE_BOARD_INDEX_2,
    RELAY_MODULE_BOARD_INDEX_3,
    RELAY_MODULE_NUM_OF_RELAY_BOARDS    // max 8 at current implementation
};
typedef enum
{
  INVALID = 0,
  TURN_ON = 0x01,
  TURN_OFF = 0x02,
  SELF_LOCKING = 0x03,
  INTERLOCK = 0x04,
  JOG = 0x05,
  DELAY = 0x06,
  TURN_ON_ALL = 0x07,
  TURN_OFF_ALL = 0x08,
} relayCmdType;

void RelayControl_init(void);
void Rte_RelayControl_runnable_10ms(void);

void RelayControl_parseCanMessage(uint8_t* CanData, uint32_t CanId);
void RelayControl_composeCanMessage(uint8_t* MsgData, uint32_t* CanId);

bool RelayControl_composeWaveshareModbusMessage(uint8_t ModuleIndx, uint8_t* MbMsgData, uint8_t GpioSt);

bool RelayControl_read_relaySt(uint8_t RelayIndx);
bool RelayControl_write_relaySt(uint8_t RelayIndx, bool state);

#endif