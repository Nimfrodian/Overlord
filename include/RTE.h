#ifndef RTE_H
#define RTE_H

#include "driver/gpio.h"
#include "driver/twai.h"

#include "RelayControl.h"
#include "ComModbus.h"
#include "ComCan.h"
#include "SDM120M.h"
#include "Dio_Cfg.h"
#include "Cli.h"

const int node_id = 1;

// GPIO section
extern void Rte_Dio_init(void);
extern bool Rte_Dio_read_gpioSt(uint8_t gpioIndx);
extern void Rte_Dio_runnable_10ms(void);

// Relay section
extern void Rte_RelayControl_init(void);
extern void Rte_RelayControl_runnable_10ms(void);
extern bool Rte_Relay_read_relaySt(uint8_t RelayIndx);
extern bool Rte_Relay_write_relaySt(uint8_t RelayIndx, bool state);

// Power meter section
extern void Rte_Sdm120m_init(void);
extern void Rte_Sdm120m_runnable_10ms(void);
extern const char* Rte_Sdm120m_read_dataName(uint8_t VarIndx);
extern uint16_t Rte_Sdm120m_read_moduleId(uint8_t ModuleIndx);

// CLI
extern void Rte_Cli_init(void);
extern void Rte_Cli_run(void);

#endif