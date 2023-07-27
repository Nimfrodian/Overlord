#ifndef COMMUNICATION_MODBUS_h
#define COMMUNICATION_MODBUS_h

#include "driver/gpio.h"
#include "driver/uart.h"
#include "ComCfg.h"

#define COM_MODBUS2_TASK_DELAY_TIME_MS  (10u)
#define COM_MODBUS0_TASK_DELAY_TIME_MS   (1u)

#define COM_MODBUS_0_BAUDRATE             (256000)
#define COM_MODBUS_2_BAUDRATE               (9600u)

const uint8_t MODBUS_TIMEOUT_COUNTER_MAX_VAL = 1;   // number of times a modbus message is re-sent if no response is received

const gpio_num_t DE_RE_PIN_MB2 = GPIO_NUM_23;  // Modbus 2 DE/RE pin (Modbus on UART2 channel)
const gpio_num_t DE_RE_PIN_MB0 = GPIO_NUM_22;  // Modbus 0 DE/RE pin (Modbus on UART0 channel)

void ComModbus_init(void);
void ComModbus_0_transceive(void* param);
void ComModbus_2_transceive(void* param);

bool ComModbus_0_writeMsg(uint16_t msgIndx, uint8_t* ModWriteBuffer, uint8_t DataTxSize);
bool ComModbus_2_writeMsg(uint16_t msgIndx, uint8_t* ModWriteBuffer, uint8_t DataTxSize, uint8_t DataRxSize);


#endif