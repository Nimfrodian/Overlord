
/*
 * Direct Memory Access Server module
 * Module is used in conjunction with Real Time Database (RTDB) module to access RAM memory during runtime
 * Supports streaming of up to 'DMAS_MAX_NUM_OF_STREAMABLE_VARS_U8'
 */

#pragma once
#include "rtdb_vars.h"


#define DMAS_API_INIT_U32                       ((tU32) 1)
#define DMAS_API_RUN_U32                        ((tU32) 2)
#define DMAS_API_REQREADYDATA_U32               ((tU32) 3)
#define DMAS_API_CAN_PARSE_U32                  ((tU32) 4)

#define DMAS_ERR_WRONG_COMMAND_TYPE_U32         ((tU32) 1)
#define DMAS_ERR_WRONG_UNIT_TYPE_U32            ((tU32) 2)
#define DMAS_ERR_RTDB_WRITE_ERR_U32             ((tU32) 3)
#define DMAS_ERR_RTDB_OVERWRITE_ERR_U32         ((tU32) 4)
#define DMAS_ERR_RTDB_RELEASEOVERWRITE_ERR_U32  ((tU32) 5)
#define DMAS_ERR_ADD_EXISTING_STREAM_U32        ((tU32) 6)
#define DMAS_ERR_REMOVE_NONEXISTING_STREAM_U32  ((tU32) 7)
#define DMAS_ERR_WRONG_CAN_ID_U32               ((tU32) 8)
#define DMAS_ERR_DATA_OVERWRITE_U32             ((tU32) 9)
#define DMAS_ERR_MODE_OUT_OF_BOUNDS_U32         ((tU32) 10)
#define DMAS_ERR_VAR_TYPE_OUT_OF_BOUNDS_U32     ((tU32) 11)
#define DMAS_ERR_TIME_INDEX                     ((tU32) 12)



#define DMAS_MAX_NUM_OF_STREAMABLE_VARS_U8      ((tU8) 16)
#define DMAS_TI_MS_MIN_STREAMING_TIME_U32       ((tU8) 10)

typedef enum
{
    DMAS_MODE_READ_REQ = 0,     ///< Request to stream variable
    DMAS_MODE_STOP_READ,        ///< Request to stop streaming variable
    DMAS_MODE_WRITE,            ///< Request to one-time write to variable
    DMAS_MODE_OVERWRITE,        ///< Request to overwrite to variable, preventing program from updating variable
    DMAS_MODE_STOPOVERWRITE,    ///< Request to release overwrite lock to allow program to take over

    DMAS_NUM_OF_MODES,
} tDMAS_MODETYPE_E;

typedef enum
{
    DMAS_TU8= 0,
    DMAS_TU16,
    DMAS_TU32,

    DMAS_TS8,
    DMAS_TS16,
    DMAS_TS32,

    DMAS_TF32S,
    DMAS_TBS,
    DMAS_TES,

    DMAS_UNDEFINED,
} tDMAS_VARTYPE_E;

enum
{
    DMAS_CMD_MODE_BYTE = 0,                 // Command Mode selection byte as per tDMAS_MODETYPE_E
    DMAS_CMD_READWRITE_VAR_TYPE = 1,        // Command Variable Type to be written byte as per tDMAS_VARTYPE_E
    DMAS_CMD_VAR_INDEX = 2,                 // Command Variable Index to read/write
    DMAS_CMD_STREAM_MS_INTERVAL_INDEX = 3,  // Command Time Interval in ms

    DMAS_RES_READWRITE_VAR_TYPE = 1,        // Response variable type
    DMAS_RES_VAR_INDEX = 2,                 // Response variable index

    DMAS_PAYLOAD_0 = 4,
    DMAS_PAYLOAD_1,
    DMAS_PAYLOAD_2,
    DMAS_PAYLOAD_3,
};

typedef struct
{
    uint32_t nr_moduleId_U32;   ///< ID of the module
} tDMAS_INITDATA_STR;

typedef struct
{
    tU8 payload_U8[8];     ///< CAN message payload to be parsed
    tU8 dataReady_U8;      ///< new data is ready to be parsed flag
} tDMAS_INPUTDATA_STR;

typedef enum
{
    DMAS_DO_NOT_STREAM = 0,
    DMAS_STREAM
} tDMAS_STREAMINGSTATETYPE_E;

typedef enum
{
    DMAS_STREAM_10ms = 0,
    DMAS_STREAM_50ms,
    DMAS_STREAM_100ms,
    DMAS_STREAM_500ms,
    DMAS_STREAM_1000ms,
    DMAS_STREAM_5000ms,
    DMAS_NUM_OF_STREAMABLE_TIMES,
} tDMAS_STREAMINGTIMETYPE_E;

typedef struct tDMAS_STREAMING_STR
{
    tU32 varIndex_U32;                              ///< variable index to stream, as defined in rtdb_vars.h
    tDMAS_VARTYPE_E varType_U32;                    ///< variable type (tU8, tU16...)
    tDMAS_STREAMINGTIMETYPE_E ti_x_varInterval_U32; ///< Streaming interval
    tU32 ti_ms_timeCounter_U32;                     ///< increments time since last read
    tDMAS_STREAMINGSTATETYPE_E streaming_U8;        ///< streaming state

    // Equality operator to check if two streaming datas are the same
    bool operator==(const tDMAS_STREAMING_STR& other) const {
        return (varIndex_U32 == other.varIndex_U32) && (varType_U32 == other.varType_U32);
    }
} tDMAS_STREAMING_STR;

typedef struct
{
    // empty
} tDMAS_OUTPUTDATA_STR;

typedef struct
{
    tU8 isReady_U8;    ///< true if data is ready, false if nothing to send
    tU8 payload_U8[8]; ///< raw CAN payload
} tDMAS_MESSAGEDATA_STR;


void dmas_init(tDMAS_INITDATA_STR* dmasCfg);
void dmas_run_5ms(void);
void dmas_canMsgParse_ev(uint8_t* DataPtr, uint32_t* MsgIdPtr);
tDMAS_MESSAGEDATA_STR dmas_getReadyData(void);