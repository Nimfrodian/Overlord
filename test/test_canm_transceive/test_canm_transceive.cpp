#include "test_helper.h"
#include <unity.h>
#include "canm.h"
#include "rtdb.h"
#include "mdll.h"

#define TEST_ti_ms_TOTAL_EXECUTION_TIME_U32 ((tU32) 10000)

static tB (send)(const tCANM_MSG *msg){return NO_ERROR;};
static tB (receive)(tCANM_MSG *msg){return NO_ERROR;};

void test_canm_init_transceive(void)
{
    tCANM_INITDATA_STR canmInitCfg_str = {
    .ti_ms_taskDelay_U32  = TEST_ti_ms_EXECUTION_TIME_U32,
    .canm_send_callback = send,
    .canm_receive_callback = receive,
    };
    tCANM_INITDATA_STR* canmInitCfg_pstr = &canmInitCfg_str;

    canm_init(canmInitCfg_pstr);


    for (tU32 ti_ms_executionTime_U32  = 0;
              ti_ms_executionTime_U32 <= TEST_ti_ms_TOTAL_EXECUTION_TIME_U32;
              ti_ms_executionTime_U32 += TEST_ti_ms_EXECUTION_TIME_U32)
    {
        canm_transceive_run_5ms();
    }


    tERRH_ERRORDATA_STR expectedError = noErrorCode;
    test_errh_isLastErrorAsExpected(expectedError);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_canm_init_transceive);
    UNITY_END();

    return 0;
}