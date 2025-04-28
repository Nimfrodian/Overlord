#ifndef CANM_RTDB_VARS_H
#define CANM_RTDB_VARS_H

#include "rtdb.h"

/*
 * note; min, default and max can be left empty if their value is not limited or relevant
 * [varType]    [arrName]                              [SIZE (optional)]; ///< [unit]  [min]  [default]  [max]    [comment]
 */

tBS      rtdb_canm_s_rxRelayInvertReq_aB   [128]; ///< []  [0]  [0]  [1]      [Relay invert requests]
tU16S    rtdb_canm_x_rxActDimmVal_aU16     [32];  ///< []  [0]  [0]  [1000]   [Actual dimmer value in 0.1 prcnt as received on CAN]

tBS      rtdb_canm_s_txGpioStates_aB       [32];  ///< []  [0]  [0]  [1]      [State of GPIOs]
tBS      rtdb_canm_s_txRelayStates_aB      [128]; ///< []  [0]  [0]  [1]      [State of relays]

#endif