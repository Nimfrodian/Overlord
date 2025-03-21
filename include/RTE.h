#ifndef RTE_H
#define RTE_H

#include "driver/gpio.h"
#include "driver/twai.h"

#include "mdll.h"
#include "RelayControl.h"
#include "ComModbus.h"
#include "ComCan.h"
#include "SDM120M.h"
#include "Dio_Cfg.h"
#include "sera.h"

const int node_id = 1;

// GPIO section
extern void Rte_Dio_init(void);
extern uint8_t Rte_Dio_read_gpioSt(uint8_t gpioIndx);
extern void Rte_Dio_runnable_10ms(void);

// Relay section
extern void Rte_RelayControl_init(void);
extern void Rte_RelayControl_runnable_10ms(void);
extern uint8_t Rte_Relay_read_relaySt(uint8_t RelayIndx);
extern uint8_t Rte_Relay_write_relaySt(uint8_t RelayIndx, uint8_t state);

// Power meter section
extern void Rte_Sdm120m_init(void);
extern void Rte_Sdm120m_runnable_10ms(void);
extern const char* Rte_Sdm120m_read_dataName(uint8_t VarIndx);
extern uint16_t Rte_Sdm120m_read_moduleId(uint8_t ModuleIndx);

#endif