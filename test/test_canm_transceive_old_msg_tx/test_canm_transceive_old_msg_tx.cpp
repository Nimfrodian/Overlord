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

static void check_old_msg_tx(void)
{
    tU32 msgsSentPre = totalMsgsSent;
    canm_x_flagCanMsgForTx((tCANM_CANMSGINDX_E) 0);
    UNITY_TEST_ASSERT_EQUAL_UINT8(1, canm_pstr_readCanMsgData((tCANM_CANMSGINDX_E) 0)->canRdyForTx_tB, __LINE__, "Message was not marked for Tx!");
    run_canm_runnable();
    UNITY_TEST_ASSERT_EQUAL_UINT8(0, canm_pstr_readCanMsgData((tCANM_CANMSGINDX_E) 0)->canRdyForTx_tB, __LINE__, "Message's Tx flag was not cleared!");
    UNITY_TEST_ASSERT_EQUAL_UINT8(msgsSentPre+1, totalMsgsSent, __LINE__, "Message was not sent!");

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

    RUN_TEST(check_old_msg_tx);

    UNITY_END();

    return 0;
}