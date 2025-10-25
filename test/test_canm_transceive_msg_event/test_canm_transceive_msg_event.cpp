#include "test_helper.h"
#include <unity.h>
#include "canm.h"
#include "rtdb.h"
#include "mdll.h"
#include <vector>
#include <cstdio>
using namespace std;

#define TEST_ti_ms_TOTAL_EXECUTION_TIME_U32 ((tU32) 50000)

static tU32 totalMsgsSent = 0;
static tU32 totalTimeRun_ms = 0;
static vector<tCANM_MSG> messages = {};

static tB (send)(const tCANM_MSG *msg)
{
    messages.push_back(*msg);
    totalMsgsSent++;
    return NO_ERROR;
};
static tB (receive)(tCANM_MSG *msg){return NO_ERROR;};

static tU32 run_canm_runnable(void)
{
    canm_transceive_run_5ms();
    totalTimeRun_ms += TEST_ti_ms_MAIN_FUNC_CYCLE_TIME_U32;
    return totalTimeRun_ms;
}

static tS32 verify_message_ID_in_vector(uint32_t msgId)
{
    for (size_t i = 0; i < messages.size(); ++i)
    {
        if (messages[i].identifier == msgId)
        {
            return static_cast<tS32>(i);
        }
    }
    return -1; // Not found
}

static void check_event_msg_tx()
{
    for (tU8 i = 0; i < 10; i++) // TODO: update 10 dimmer outputs
    {
        rtdb_write_tBS((tBSEnumT)(RTDB_DIMH_S_TXDIMMERRDYFLAG_AB_0 + i), 1);
        rtdb_write_tU16S((tU16SEnumT) (RTDB_DIMH_X_TXDIMMERVAL_AU16_0 + i), (0x5A + i));
        rtdb_write_tU32S((tU32SEnumT) (RTDB_DIMH_TI_MS_TXDIMMERTIME_AU32_0 + i), (0xA5) + i);

        run_canm_runnable();

        tS32 msgPos = verify_message_ID_in_vector(0x95);
        UNITY_TEST_ASSERT_EQUAL_UINT8(true, msgPos >= 0, __LINE__, "Dimmer message missing!");
        tU16 dimmTime_U32 = (messages[msgPos].data[0] << 0) | (messages[msgPos].data[1] << 8) | (messages[msgPos].data[2] << 16);
        tU16 dimmVal_U16 = (messages[msgPos].data[3] << 0) | (messages[msgPos].data[4] << 8);
        tU16 dimmIndx = (messages[msgPos].data[5] << 0) | (messages[msgPos].data[6] << 8) | (messages[msgPos].data[7] << 16);
        UNITY_TEST_ASSERT_EQUAL_UINT32(rtdb_read_tU16S((tU16SEnumT) (RTDB_DIMH_X_TXDIMMERVAL_AU16_0 + i)), dimmVal_U16,  __LINE__, "Dimmer value was not properly packaged!");
        UNITY_TEST_ASSERT_EQUAL_UINT32(rtdb_read_tU32S((tU32SEnumT) (RTDB_DIMH_TI_MS_TXDIMMERTIME_AU32_0 + i)), dimmTime_U32, __LINE__, "Dimmer time was not properly packaged!");
        UNITY_TEST_ASSERT_EQUAL_UINT32((0x01 << i), dimmIndx,    __LINE__, "Dimmer index was not properly packaged!");
        messages.clear();
    }

    tERRH_ERRORDATA_STR expectedError = noErrorCode;
    test_errh_isLastErrorAsExpected(expectedError);
}


int main(int argc, char **argv) {
    UNITY_BEGIN();

    tCANM_INITDATA_STR canmInitCfg_str = {
    .ti_ms_taskDelay_U32  = TEST_ti_ms_MAIN_FUNC_CYCLE_TIME_U32,
    .canm_send_callback = send,
    .canm_receive_callback = receive,
    };
    tCANM_INITDATA_STR* canmInitCfg_pstr = &canmInitCfg_str;
    canm_init(canmInitCfg_pstr);

    RUN_TEST(check_event_msg_tx);
    UNITY_END();

    return 0;
}