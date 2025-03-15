/*
*
*   Error handling module
*
*/
#pragma once

#include "stdint.h"
#include "timh.h"

#define ERRH_API_INIT_U32                       ((uint32_t) 1)
#define ERRH_API_READ_ERROR_U32                 ((uint32_t) 2)
#define ERRH_API_CAN_COMPOSE_U32                ((uint32_t) 3)

#define ERRH_ERR_READ_INDEX_OUT_OF_BOUNDS_U32   ((uint32_t) 1)



#define ERRH_NR_ERROR_BUFFER_SIZE_U32 ((uint32_t) 64)   ///< number of spots for errors. First come first served. Should be large enough for all errors


typedef struct
{
    uint32_t nr_moduleId_U32;       ///< ID of the module
} tERRH_INITDATA_STR;

typedef enum
{
    ERRH_ERRORTYPE_UNDEF,           ///< shall not be used
    ERRH_ERROR_CRITICAL,        ///< System will stop to prevent further damage
    ERRH_ERROR_HIGH,            ///< System will take corrective action and apply limitations to prevent damage
    ERRH_ERROR_LOW,             ///< System will apply limitations to prevent damage
    ERRH_WARNING,               ///< System will notify other systems and/or user of a warning
    ERRH_NOTIF,                 ///< System will only take notice of unwanted behaviour

    ERRH_COUNT
} tERRH_ERRORTYPE_E;

typedef enum
{
    ERRH_COMMONERRORTYPE_UNDEF = 0,
    ERRH_MODULE_NOT_INIT,
    ERRH_MODULE_ALREADY_INIT,
    ERRH_POINTER_IS_NULL,
} tERRH_COMMONERROR_E;

typedef struct
{
    uint32_t moduleId;                  ///< ID of the module from which error is called
    uint32_t instanceId;                ///< ID of the instance
    uint32_t apiId;                     ///< ID of the API (function) in which error is triggered
    uint32_t errorId;                   ///< ID of the error triggered
    tERRH_ERRORTYPE_E errorLvl : 8;     ///< Level of severity
    tTIMH_TIMEDATA_STR ti_globalTime;   ///< Actual world time at error event
    int64_t ti_us_timestamp;            ///< timestamp of error in us from program start
    uint8_t s_sentOnCan_U8;             ///< true if error was already broadcasted on CAN since last change
    uint8_t count_U8;                   ///< number of times this error has been triggered. Caps at 255
} tERRH_ERRORDATA_STR;

/**
 * @brief Function initializes error handling module
 * @param void
 * @return (void)
 */
void errh_init(tERRH_INITDATA_STR* ErrhCfg);

/**
 * @brief Function de-initializes error handling module
 * @param void
 * @return (void)
 */
void errh_deinit(void);

/**
 * @brief Function for error reporting
 * @param errorLvl Level of severity for given error
 * @param moduleId module ID as defined in its header file
 * @param instanceId instance ID
 * @param apiId API ID, to distinguish function in which it is triggered
 * @param errorId Specific ID of the error
 * @return (void)
 */
void errh_reportError(tERRH_ERRORTYPE_E ErrorLvl, uint32_t ModuleId, uint32_t InstanceId, uint32_t ApiId, uint32_t ErrorId);

/**
 * @brief Function returns the error data of error index. Used for iterating
 * @param ErrorIndx index of the error to be read from error buffer
 * @return error data for indexed error
 */
tERRH_ERRORDATA_STR errh_readError(uint32_t ErrorIndx);

/**
 * @brief Function returns the number of (unique) reported errors
 * @param void
 * @return number of (unique) reported errors
 */
uint32_t errh_readReportedErrorCount(void);

/**
 * @brief Function clears error counting
 * @param
 * @return (void)
 */
void errh_clearErrorCount(void);

/**
 * @brief Function fills out 4 CAN messages with a single error data
 * @param DataPtr_MpU8 a matrix of 4 pointers to 8 bytes of memory for 4 64bit CAN messages
 * @return (void)
 */
bool errh_canMsgCompose_100ms(uint8_t** DataPtr_MpU8);