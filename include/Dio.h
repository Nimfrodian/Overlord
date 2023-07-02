#ifndef DIO_H
#define DIO_H

#include "freertos/FreeRTOS.h"  // for basic data types
#include "Dio_Cfg.h"

void Dio_init(void);
void Dio_can_compose(uint8_t* CanDataPtr, uint32_t* CanIdPtr, bool* GpioStArr);
void Dio_can_parse(uint8_t* CanData);

uint16_t Dio_read_gpioOutReqSt(BSW_Dio_gpioOutIndxType outIndx);

#endif