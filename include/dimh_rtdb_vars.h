#ifndef DIMH_RTDB_VARS_H
#define DIMH_RTDB_VARS_H

#include "rtdb.h"

#define DIMH_TI_MS_DEF_DIMMING_TIME_U32 ((tU32) 1000)
#define DIMH_DEF_DIMMING_MIN_U32        ((tU32)    0)
#define DIMH_DEF_DIMMING_DEF_U32        ((tU32)  500)
#define DIMH_DEF_DIMMING_MAX_U32        ((tU32) 1000)

/*
 * note; min, default and max can be left empty if their value is not limited or relevant
 * [varType]    [arrName]                              [SIZE (optional)]; ///< [unit]  [min]  [default]  [max]    [comment]
 */

tU16S    rtdb_dimh_x_dimmerTrshld_aU16       [32]; ///< []    [DIMH_DEF_DIMMING_MIN_U32]  [DIMH_DEF_DIMMING_MIN_U32]         [DIMH_DEF_DIMMING_MAX_U32]   [Dimmer treshold. If actual value is below this treshold the dimmer is considered OFF]
tU16S    rtdb_dimh_x_dimmerOn_aU16           [32]; ///< []    [DIMH_DEF_DIMMING_MIN_U32]  [DIMH_DEF_DIMMING_DEF_U32]         [DIMH_DEF_DIMMING_MAX_U32]   [Dimmer ON value. When dimmer is set to ON this value is used to set the dimmer]
tU16S    rtdb_dimh_x_txDimmerVal_aU16        [32]; ///< []    [DIMH_DEF_DIMMING_MIN_U32]  [DIMH_DEF_DIMMING_MIN_U32]         [DIMH_DEF_DIMMING_MAX_U32]   [Dimmer value in 0.1 prcnt]

tU32S    rtdb_dimh_ti_ms_txDimmerTime_aU32   [32]; ///< [ms]  [0]                         [DIMH_TI_MS_DEF_DIMMING_TIME_U32]  []                           [Dimming time in ms for both on and off]

tBS      rtdb_dimh_s_txDimmerRdyFlag_aB      [32]; ///< []    [0]                         [0]                                [1]                          [Dimmer data ready flag. Set by DIMH, cleared by CANM]
#endif