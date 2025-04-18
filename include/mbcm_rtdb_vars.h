#ifndef MBCM_RTDB_VARS_H
#define MBCM_RTDB_VARS_H

#include "rtdb.h"

#define MBCM_TI_US_MIN_TASK_TIME_RELAY_U32 ((uint32_t) 10)          //  10 us between queries
#define MBCM_TI_US_DEF_TASK_TIME_RELAY_U32 ((uint32_t) 2000)        //   2 ms between queries
#define MBCM_TI_US_MAX_TASK_TIME_RELAY_U32 ((uint32_t) 100000)      //  100 ms between queries


#define MBCM_TI_US_MIN_TASK_TIME_PWRMETER_U32 ((uint32_t) 10)       //  10 us between queries
#define MBCM_TI_US_DEF_TASK_TIME_PWRMETER_U32 ((uint32_t) 50000)    //  50 ms between queries
#define MBCM_TI_US_MAX_TASK_TIME_PWRMETER_U32 ((uint32_t) 1000000)  //   1 s between queries

/*
 * note; min, default and max can be left empty if their value is not limited or relevant
 * [varType]    [arrName]                              [SIZE (optional)]; ///< [unit]  [min]  [default]  [max]    [comment]
 */
tU32S    rtdb_mbcm_ti_us_taskPeriodRelay_U32;           ///< [us]  [MBCM_TI_US_MIN_TASK_TIME_RELAY_U32]     [MBCM_TI_US_DEF_TASK_TIME_RELAY_U32]     [MBCM_TI_US_MAX_TASK_TIME_RELAY_U32]      [Time in us between Modbus messages for Relay modules]
tU32S    rtdb_mbcm_ti_us_taskPeriodPwrMeter_U32;        ///< [us]  [MBCM_TI_US_MIN_TASK_TIME_PWRMETER_U32]  [MBCM_TI_US_DEF_TASK_TIME_PWRMETER_U32]  [MBCM_TI_US_MAX_TASK_TIME_PWRMETER_U32]   [Time in us between Modbus messages for Power Meter modules]

tU8S     rtdb_mbcm_x_desiredRelayStates_aU8       [16]; ///< []    [0]                                      [0]                                      [0xFF]                                    [Desired relay states as 8 bit mask, 1 for each relay module. Max 16 relay modules for a total of 128 relays]
tU8S     rtdb_mbcm_x_actualRelayStates_aU8        [16]; ///< []    [0]                                      [0]                                      [0xFF]                                    [Actual relay states as 8 bit mask, 1 for each relay module. Max 16 relay modules for a total of 128 relays]

tF32S    rtdb_mbcm_x_sdm120mReadings_0_aF32       [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 0]
tF32S    rtdb_mbcm_x_sdm120mReadings_1_aF32       [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 1]
tF32S    rtdb_mbcm_x_sdm120mReadings_2_aF32       [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 2]
tF32S    rtdb_mbcm_x_sdm120mReadings_3_aF32       [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 3]
tF32S    rtdb_mbcm_x_sdm120mReadings_4_aF32       [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 4]
tF32S    rtdb_mbcm_x_sdm120mReadings_5_aF32       [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 5]
tF32S    rtdb_mbcm_x_sdm120mReadings_6_aF32       [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 6]
tF32S    rtdb_mbcm_x_sdm120mReadings_7_aF32       [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 7]
tF32S    rtdb_mbcm_x_sdm120mReadings_8_aF32       [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 8]
tF32S    rtdb_mbcm_x_sdm120mReadings_9_aF32       [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 9]
tF32S    rtdb_mbcm_x_sdm120mReadings_10_aF32      [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 10]
tF32S    rtdb_mbcm_x_sdm120mReadings_11_aF32      [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 11]
tF32S    rtdb_mbcm_x_sdm120mReadings_12_aF32      [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 12]
tF32S    rtdb_mbcm_x_sdm120mReadings_13_aF32      [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 13]
tF32S    rtdb_mbcm_x_sdm120mReadings_14_aF32      [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 14]
tF32S    rtdb_mbcm_x_sdm120mReadings_15_aF32      [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 15]
tF32S    rtdb_mbcm_x_sdm120mReadings_16_aF32      [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 16]
tF32S    rtdb_mbcm_x_sdm120mReadings_17_aF32      [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 17]
tF32S    rtdb_mbcm_x_sdm120mReadings_18_aF32      [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 18]
tF32S    rtdb_mbcm_x_sdm120mReadings_19_aF32      [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 19]
tF32S    rtdb_mbcm_x_sdm120mReadings_20_aF32      [21]; ///< []    []                                       [0]                                      []                                        [SDM120M readings as 32 bit float, for module 20]

#endif