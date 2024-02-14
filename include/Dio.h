#ifndef DIO_H
#define DIO_H

#include <stdint.h>  // for basic data types
#include "Dio_Cfg.h"

void Dio_init(void);
void Dio_can_compose(uint8_t* CanDataPtr, uint32_t* CanIdPtr, bool* GpioStArr);
void Dio_can_parse(uint8_t* CanData);

#endif