#ifndef RTE_H
#define RTE_H

#include "driver/gpio.h"
#include <ESP32CAN.h>

#include "RelayControl.h"
#include "ComModbus.h"
#include "ComCan.h"
#include "SDM120M.h"
#include "GpioControl.h"

const int node_id = 1;

// Relay section
void Rte_RelayControl_init(void);
void Rte_RelayControl_run_10ms(void);

// Power meter section
void Rte_Sdm120m_init(void);
void Rte_Sdm120m_run_10ms(void);

#endif