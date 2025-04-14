#ifndef CANM_RTDB_VARS_H
#define CANM_RTDB_VARS_H

#include "rtdb.h"

/*
 * note; min, default and max can be left empty if their value is not limited or relevant
 * [varType]    [arrName]                              [SIZE (optional)]; ///< [unit]  [min]  [default]  [max]    [comment]
 */

tBS    canm_s_rxRelayInvertReq_aB   [128]; ///< []  [0]  [0]  [1]   [Relay invert requests]

tBS    canm_s_txGpioStates_aB       [32];  ///< []  [0]  [0]  [1]   [State of GPIOs]
tBS    canm_s_txRelayStates_aB      [128]; ///< []  [0]  [0]  [1]   [State of relays]

#endif