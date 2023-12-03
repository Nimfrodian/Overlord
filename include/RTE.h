#ifndef RTE_H
#define RTE_H

#include "driver/gpio.h"
#include <ESP32CAN.h>

#include "RelayControl.h"
#include "ComModbus.h"
#include "ComCan.h"
#include "SDM120M.h"
#include "Dio_Cfg.h"
#include "Cli.h"

const int node_id = 1;

// GPIO section
extern void Rte_Dio_init(void);
extern bool Rte_Dio_get_gpioSt(uint8_t gpioIndx);
extern bool Rte_Dio_set_gpioDbgSt(bool GpioSts, uint8_t GpioDbgIndx);
extern void Rte_Dio_runnable_10ms(void);

// Relay section
extern void Rte_RelayControl_init(void);
extern void Rte_RelayControl_runnable_10ms(void);

// Power meter section
extern void Rte_Sdm120m_init(void);
extern void Rte_Sdm120m_runnable_10ms(void);

// CLI
extern void Rte_Cli_init(void);
extern void Rte_Cli_run(void);

#endif