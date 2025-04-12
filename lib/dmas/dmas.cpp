#include "dmas.h"
#include "errh.h"
#include "rtdb.h"
#include <vector>
#include <algorithm> // for std::find_if

static bool dmas_s_moduleInit_tB = false;
static uint32_t dmas_nr_moduleId_U32 = 0;

using namespace std;
static vector<tDMAS_STREAMING_STR> dmas_x_streamingData_astr(DMAS_MAX_NUM_OF_STREAMABLE_VARS_U8);
tDMAS_STREAMING_STR dmas_x_nullData_str = {
                .varIndex_U32 = 0,
                .varType_U32 = DMAS_UNDEFINED,
                .ti_x_varInterval_U32 = (tDMAS_STREAMINGTIMETYPE_E) 0,
                .ti_ms_timeCounter_U32 = 0,
                .streaming_U8= DMAS_DO_NOT_STREAM,
            };

static tDMAS_INPUTDATA_STR dmas_x_inputData_str = {
    .payload_U8 = {0},
    .dataReady_U8 = false,
};

// Helper functions
static tU32 dmas_ti_ms_streamableTimes_aU32[DMAS_NUM_OF_STREAMABLE_TIMES] =
{
    [DMAS_STREAM_10ms] = 10,
    [DMAS_STREAM_50ms] = 50,
    [DMAS_STREAM_100ms] = 100,
    [DMAS_STREAM_500ms] = 500,
    [DMAS_STREAM_1000ms] = 1000,
    [DMAS_STREAM_5000ms] = 5000
};

tU32 dmas_U32_ms_getStreamingTime( tDMAS_STREAMINGTIMETYPE_E StreamingTimeSelection_E)
{
    tU32 ti_ms_toReturn_U32 = dmas_ti_ms_streamableTimes_aU32[DMAS_NUM_OF_STREAMABLE_TIMES - 1];
    if (DMAS_NUM_OF_STREAMABLE_TIMES > StreamingTimeSelection_E)
    {
        ti_ms_toReturn_U32 = dmas_ti_ms_streamableTimes_aU32[StreamingTimeSelection_E];
    }
    else
    {
        errh_reportError(ERRH_ERROR_LOW, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_TIME_INDEX);
    }
    return ti_ms_toReturn_U32;
}

// Module functions

void dmas_init(tDMAS_INITDATA_STR* DmasCfg)
{
    if (true == dmas_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == DmasCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, dmas_nr_moduleId_U32, 0, DMAS_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        dmas_nr_moduleId_U32 = DmasCfg->nr_moduleId_U32;
        dmas_s_moduleInit_tB = true;    // only init once

        for (int i = 0; i < DMAS_MAX_NUM_OF_STREAMABLE_VARS_U8; ++i)
        {
            dmas_x_streamingData_astr[i] = dmas_x_nullData_str;
        }
    }
}

/**
 * @brief Runnable for DMAS module which allows for internal variable streaming and setting
 * @param void
 * @return (void)
 */
void dmas_run_5ms(void)
{
    if (true == dmas_x_inputData_str.dataReady_U8)
    {
        switch( dmas_x_inputData_str.payload_U8[DMAS_CMD_MODE_BYTE] )
        {
            case DMAS_MODE_READ_REQ:
            {
                tDMAS_STREAMING_STR newReq_str = {
                    .varIndex_U32 = dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX],
                    .varType_U32 = (tDMAS_VARTYPE_E) dmas_x_inputData_str.payload_U8[DMAS_CMD_READWRITE_VAR_TYPE],
                    .ti_x_varInterval_U32 = (tDMAS_STREAMINGTIMETYPE_E) dmas_x_inputData_str.payload_U8[DMAS_CMD_STREAM_MS_INTERVAL_INDEX],
                    .ti_ms_timeCounter_U32 = 0,//dmas_U32_ms_getStreamingTime((tDMAS_STREAMINGTIMETYPE_E) dmas_x_inputData_str.payload_U8[DMAS_CMD_STREAM_MS_INTERVAL_INDEX]), // start immediately
                    .streaming_U8= DMAS_STREAM,
                };

                // Check if the new request already exists in the vector
                auto it = std::find_if(dmas_x_streamingData_astr.begin(), dmas_x_streamingData_astr.end(), [&newReq_str](const tDMAS_STREAMING_STR& p) {
                    return p == newReq_str;
                });

                if (it != dmas_x_streamingData_astr.end())
                {
                    errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_ADD_EXISTING_STREAM_U32);
                }
                else
                {
                    // Add the new point if it doesn't exist
                    dmas_x_streamingData_astr.push_back(newReq_str);
                }
                break;
            }
            case DMAS_MODE_STOP_READ:
            {
                tDMAS_STREAMING_STR newReq_str = {
                    .varIndex_U32 = dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX],
                    .varType_U32 = (tDMAS_VARTYPE_E) dmas_x_inputData_str.payload_U8[DMAS_CMD_READWRITE_VAR_TYPE],
                    .ti_x_varInterval_U32 = (tDMAS_STREAMINGTIMETYPE_E) dmas_x_inputData_str.payload_U8[DMAS_CMD_STREAM_MS_INTERVAL_INDEX],
                    .ti_ms_timeCounter_U32 = 0,
                    .streaming_U8= DMAS_STREAM,
                };

                // Check if the given request already exists in the vector
                auto it = std::find_if(dmas_x_streamingData_astr.begin(), dmas_x_streamingData_astr.end(), [&newReq_str](const tDMAS_STREAMING_STR& p) {
                    return p == newReq_str;
                });

                if (it != dmas_x_streamingData_astr.end())
                {
                    dmas_x_streamingData_astr.erase(it);
                }
                else
                {
                    errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_REMOVE_NONEXISTING_STREAM_U32);
                }
                break;
            }
            case DMAS_MODE_WRITE:
            {
                tU32 errCode = 0;
                tU8* ptr = dmas_x_inputData_str.payload_U8;
                tU32 rawNewVal_U32 = (ptr[7] << 24) | (ptr[6] << 16) | (ptr[5] << 8) | (ptr[4] << 0);
                switch (dmas_x_inputData_str.payload_U8[DMAS_CMD_READWRITE_VAR_TYPE])
                {
                    case DMAS_TU8:
                    {
                        errCode = rtdb_write_tU8S((tU8SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tU8) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TU16:
                    {
                        errCode = rtdb_write_tU16S((tU16SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tU16) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TU32:
                    {
                        errCode = rtdb_write_tU32S((tU32SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tU32) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TS8:
                    {
                        errCode = rtdb_write_tS8S((tS8SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tS8) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TS16:
                    {
                        errCode = rtdb_write_tS16S((tS16SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tS16) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TS32:
                    {
                        errCode = rtdb_write_tS32S((tS32SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tS32) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TF32S:
                    {
                        errCode = rtdb_write_tF32S((tF32SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tF32) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TBS:
                    {
                        errCode = rtdb_write_tBS((tBSEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tB) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TES:
                    {
                        errCode = rtdb_write_tES((tESEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tE) rawNewVal_U32);
                        break;
                    }
                    default:
                    {
                        errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_WRONG_UNIT_TYPE_U32);
                        break;
                    }

                    if (errCode)
                    {
                        errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_RTDB_WRITE_ERR_U32);
                    }
                }
                break;
            }
            case DMAS_MODE_OVERWRITE:
            {
                tU32 errCode = 0;
                tU8* ptr = dmas_x_inputData_str.payload_U8;
                tU32 rawNewVal_U32 = (ptr[7] << 24) | (ptr[6] << 16) | (ptr[5] << 8) | (ptr[4] << 0);
                switch (dmas_x_inputData_str.payload_U8[DMAS_CMD_READWRITE_VAR_TYPE])
                {
                    case DMAS_TU8:
                    {
                        errCode = rtdb_overwrite_tU8S((tU8SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tU8) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TU16:
                    {
                        errCode = rtdb_overwrite_tU16S((tU16SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tU16) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TU32:
                    {
                        errCode = rtdb_overwrite_tU32S((tU32SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tU32) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TS8:
                    {
                        errCode = rtdb_overwrite_tS8S((tS8SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tS8) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TS16:
                    {
                        errCode = rtdb_overwrite_tS16S((tS16SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tS16) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TS32:
                    {
                        errCode = rtdb_overwrite_tS32S((tS32SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tS32) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TF32S:
                    {
                        errCode = rtdb_overwrite_tF32S((tF32SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tF32) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TBS:
                    {
                        errCode = rtdb_overwrite_tBS((tBSEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tB) rawNewVal_U32);
                        break;
                    }
                    case DMAS_TES:
                    {
                        errCode = rtdb_overwrite_tES((tESEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX], (tE) rawNewVal_U32);
                        break;
                    }
                    default:
                    {
                        errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_WRONG_UNIT_TYPE_U32);
                        break;
                    }

                    if (errCode)
                    {
                        errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_RTDB_OVERWRITE_ERR_U32);
                    }
                }
                break;
            }
            case DMAS_MODE_STOPOVERWRITE:
            {
                tU32 errCode = 0;
                switch (dmas_x_inputData_str.payload_U8[DMAS_CMD_READWRITE_VAR_TYPE])
                {
                    case DMAS_TU8:
                    {
                        errCode = rtdb_releaseOverwrite_tU8S((tU8SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX]);
                        break;
                    }
                    case DMAS_TU16:
                    {
                        errCode = rtdb_releaseOverwrite_tU16S((tU16SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX]);
                        break;
                    }
                    case DMAS_TU32:
                    {
                        errCode = rtdb_releaseOverwrite_tU32S((tU32SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX]);
                        break;
                    }
                    case DMAS_TS8:
                    {
                        errCode = rtdb_releaseOverwrite_tS8S((tS8SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX]);
                        break;
                    }
                    case DMAS_TS16:
                    {
                        errCode = rtdb_releaseOverwrite_tS16S((tS16SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX]);
                        break;
                    }
                    case DMAS_TS32:
                    {
                        errCode = rtdb_releaseOverwrite_tS32S((tS32SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX]);
                        break;
                    }
                    case DMAS_TF32S:
                    {
                        errCode = rtdb_releaseOverwrite_tF32S((tF32SEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX]);
                        break;
                    }
                    case DMAS_TBS:
                    {
                        errCode = rtdb_releaseOverwrite_tBS((tBSEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX]);
                        break;
                    }
                    case DMAS_TES:
                    {
                        errCode = rtdb_releaseOverwrite_tES((tESEnumT) dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX]);
                        break;
                    }
                    default:
                    {
                        errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_WRONG_UNIT_TYPE_U32);
                        break;
                    }

                    if (errCode)
                    {
                        errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_RTDB_RELEASEOVERWRITE_ERR_U32);
                    }
                }
                break;
            }
            default:
            {
                errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_WRONG_COMMAND_TYPE_U32);
                break;
            }
        }
    }

    // clear input command
    dmas_x_inputData_str.dataReady_U8= false;

    // update time on streaming variables
    {
        for (vector<tDMAS_STREAMING_STR>::iterator it=dmas_x_streamingData_astr.begin(); it < dmas_x_streamingData_astr.end(); it++)
        {
            if (DMAS_STREAM == it->streaming_U8)
            {
                it->ti_ms_timeCounter_U32 += 5;
            }
        }
    }
}

/**
 * @brief Function returns a struct containing the payload to be sent and a flag if data is valid
 * @param void
 * @return struct containing payload and send-ready flag
 */
tDMAS_MESSAGEDATA_STR dmas_getReadyData(void)
{
    tDMAS_MESSAGEDATA_STR toReturn = {
        .isReady_U8 = false,
        .payload_U8 = {0}
    };

    for (vector<tDMAS_STREAMING_STR>::iterator it=dmas_x_streamingData_astr.begin(); it < dmas_x_streamingData_astr.end(); it++)
    {
        if (DMAS_STREAM == it->streaming_U8)
        {
            if (it->ti_ms_timeCounter_U32 >= dmas_U32_ms_getStreamingTime(it->ti_x_varInterval_U32))
            {
                it->ti_ms_timeCounter_U32 = 0;
                toReturn.payload_U8[DMAS_CMD_MODE_BYTE] = 0;
                toReturn.payload_U8[DMAS_CMD_READWRITE_VAR_TYPE] = it->varType_U32;
                toReturn.payload_U8[DMAS_CMD_VAR_INDEX] = it->varIndex_U32;
                toReturn.payload_U8[DMAS_CMD_STREAM_MS_INTERVAL_INDEX] = it->ti_x_varInterval_U32;


                switch (it->varType_U32)
                {
                    case DMAS_TU8:
                    {
                        tU8 readData_U8= rtdb_read_tU8S((tU8SEnumT) it->varIndex_U32);
                        toReturn.payload_U8[DMAS_PAYLOAD_0] = (readData_U8>> 0) & 0xFF;
                        break;
                    }
                    case DMAS_TU16:
                    {
                        tU16 readData_U16= rtdb_read_tU16S((tU16SEnumT) it->varIndex_U32);
                        toReturn.payload_U8[DMAS_PAYLOAD_0] = (readData_U16>> 0) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_1] = (readData_U16>> 8) & 0xFF;
                        break;
                    }
                    case DMAS_TU32:
                    {
                        tU32 readData_U32 = rtdb_read_tU32S((tU32SEnumT) it->varIndex_U32);
                        toReturn.payload_U8[DMAS_PAYLOAD_0] = (readData_U32 >> 0) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_1] = (readData_U32 >> 8) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_2] = (readData_U32 >> 16) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_3] = (readData_U32 >> 24) & 0xFF;
                        break;
                    }
                    case DMAS_TS8:
                    {
                        tS8 readData_S8 = rtdb_read_tS8S((tS8SEnumT) it->varIndex_U32);
                        toReturn.payload_U8[DMAS_PAYLOAD_0] = (readData_S8 >> 0) & 0xFF;
                        break;
                    }
                    case DMAS_TS16:
                    {
                        tS16 readData_S16 = rtdb_read_tS16S((tS16SEnumT) it->varIndex_U32);
                        toReturn.payload_U8[DMAS_PAYLOAD_0] = (readData_S16 >> 0) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_1] = (readData_S16 >> 8) & 0xFF;
                        break;
                    }
                    case DMAS_TS32:
                    {
                        tS32 readData_tS32 = rtdb_read_tS32S((tS32SEnumT) it->varIndex_U32);
                        toReturn.payload_U8[DMAS_PAYLOAD_0] = (readData_tS32 >> 0) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_1] = (readData_tS32 >> 8) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_2] = (readData_tS32 >> 16) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_3] = (readData_tS32 >> 24) & 0xFF;
                        break;
                    }
                    case DMAS_TF32S:
                    {
                        tF32 readData_F32 = rtdb_read_tF32S((tF32SEnumT) it->varIndex_U32);
                        tU32 readData_U32 = *((tU32*)((void*) &readData_F32)); // convert to tU32
                        toReturn.payload_U8[DMAS_PAYLOAD_0] = ((readData_U32) >> 0) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_1] = ((readData_U32) >> 8) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_2] = ((readData_U32) >> 16) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_3] = ((readData_U32) >> 24) & 0xFF;
                        break;
                    }
                    case DMAS_TBS:
                    {
                        tU8 readData_U8= rtdb_read_tU8S((tU8SEnumT) it->varIndex_U32);
                        toReturn.payload_U8[DMAS_PAYLOAD_0] = (readData_U8>> 0) & 0x01;
                        break;
                    }
                    case DMAS_TES:
                    {
                        tU32 readData_U32 = rtdb_read_tU32S((tU32SEnumT) it->varIndex_U32);
                        toReturn.payload_U8[DMAS_PAYLOAD_0] = (readData_U32 >> 0) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_1] = (readData_U32 >> 8) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_2] = (readData_U32 >> 16) & 0xFF;
                        toReturn.payload_U8[DMAS_PAYLOAD_3] = (readData_U32 >> 24) & 0xFF;
                        break;
                    }
                    default:
                    {
                        errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_WRONG_UNIT_TYPE_U32);
                        break;
                    }
                }
                toReturn.isReady_U8 = true;
            }
        }
    }

    return toReturn;
}

void dmas_canMsgParse_ev(uint8_t* DataPtr, uint32_t* MsgIdPtr)
{
    if (false == dmas_s_moduleInit_tB)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, dmas_nr_moduleId_U32, 0, DMAS_API_CAN_PARSE_U32, ERRH_MODULE_NOT_INIT);
    }
    else
    {
        tU32 modeType_U32 = DataPtr[DMAS_CMD_MODE_BYTE];
        tU32 varType_U32 = *MsgIdPtr - 0x10;
        tU32 streamIntervalIndex_U32 = DataPtr[DMAS_CMD_STREAM_MS_INTERVAL_INDEX];

        if (true == dmas_x_inputData_str.dataReady_U8)
        {
            errh_reportError(ERRH_WARNING, dmas_nr_moduleId_U32, 0, DMAS_API_CAN_PARSE_U32, DMAS_ERR_DATA_OVERWRITE_U32);
        }

        if (DMAS_NUM_OF_MODES <= modeType_U32)
        {
            errh_reportError(ERRH_ERROR_LOW, dmas_nr_moduleId_U32, 0, DMAS_API_CAN_PARSE_U32, DMAS_ERR_MODE_OUT_OF_BOUNDS_U32);
        }
        else if (DMAS_UNDEFINED <= varType_U32)
        {
            errh_reportError(ERRH_ERROR_LOW, dmas_nr_moduleId_U32, 0, DMAS_API_CAN_PARSE_U32, DMAS_ERR_VAR_TYPE_OUT_OF_BOUNDS_U32);
        }
        else if ((DMAS_NUM_OF_STREAMABLE_TIMES <= streamIntervalIndex_U32) && (DMAS_MODE_READ_REQ == modeType_U32))
        {
            errh_reportError(ERRH_ERROR_LOW, dmas_nr_moduleId_U32, 0, DMAS_API_CAN_PARSE_U32, DMAS_ERR_TIME_INDEX);
        }
        else
        {
            dmas_x_inputData_str.payload_U8[DMAS_CMD_MODE_BYTE] = modeType_U32;
            dmas_x_inputData_str.payload_U8[DMAS_CMD_READWRITE_VAR_TYPE] = varType_U32;
            dmas_x_inputData_str.payload_U8[DMAS_CMD_VAR_INDEX] = DataPtr[DMAS_CMD_VAR_INDEX];
            dmas_x_inputData_str.payload_U8[DMAS_CMD_STREAM_MS_INTERVAL_INDEX] = streamIntervalIndex_U32;

            dmas_x_inputData_str.payload_U8[DMAS_PAYLOAD_0] = DataPtr[DMAS_PAYLOAD_0];
            dmas_x_inputData_str.payload_U8[DMAS_PAYLOAD_1] = DataPtr[DMAS_PAYLOAD_1];
            dmas_x_inputData_str.payload_U8[DMAS_PAYLOAD_2] = DataPtr[DMAS_PAYLOAD_2];
            dmas_x_inputData_str.payload_U8[DMAS_PAYLOAD_3] = DataPtr[DMAS_PAYLOAD_3];

            dmas_x_inputData_str.dataReady_U8 = true;
        }
    }
}