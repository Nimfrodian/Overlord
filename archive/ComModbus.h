#ifndef COMMUNICATION_MODBUS_h
#define COMMUNICATION_MODBUS_h

#include "driver/gpio.h"
#include "driver/uart.h"
#include "ComCfg.h"

#define COM_MODBUS1_TASK_DELAY_TIME_MS   (1u)
#define COM_MODBUS2_TASK_DELAY_TIME_MS  (10u)

#define COM_MODBUS_1_BAUDRATE             (256000)
#define COM_MODBUS_2_BAUDRATE              (9600u)

const uint8_t MODBUS_TIMEOUT_COUNTER_MAX_VAL = 1;   // number of times a modbus message is re-sent if no response is received

const gpio_num_t DE_RE_PIN_MB1 = GPIO_NUM_11;  // Modbus 1 DE/RE pin (Modbus on UART1 channel)
const gpio_num_t DE_RE_PIN_MB2 = GPIO_NUM_12;  // Modbus 2 DE/RE pin (Modbus on UART2 channel)

void ComModbus_init(void);
void ComModbus_1_transceive(void* param);
void ComModbus_2_transceive(void* param);

uint8_t ComModbus_1_writeMsg(uint16_t msgIndx, uint8_t* ModWriteBuffer, uint8_t DataTxSize);
uint8_t ComModbus_2_writeMsg(uint16_t msgIndx, uint8_t* ModWriteBuffer, uint8_t DataTxSize, uint8_t DataRxSize);


#endif