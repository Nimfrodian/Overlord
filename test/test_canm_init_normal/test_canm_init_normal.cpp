#include "test_helper.h"
#include <unity.h>
#include "canm.h"
#include "mdll.h"

/* LCOV_EXCL_START */
static tB send(const tCANM_MSG *msg){return NO_ERROR;}
static tB receive(tCANM_MSG *msg){return NO_ERROR;}
/* LCOV_EXCL_STOP */

void test_canm_init_normal_init(void)
{
    tCANM_INITDATA_STR canmInitCfg_str = {
    .ti_ms_taskDelay_U32  = TEST_ti_ms_EXECUTION_TIME_U32,
    .canm_send_callback = send,
    .canm_receive_callback = receive,
    };
    tCANM_INITDATA_STR* canmInitCfg_pstr = &canmInitCfg_str;

    canm_init(canmInitCfg_pstr);

    tERRH_ERRORDATA_STR expectedError = noErrorCode;
    test_errh_isLastErrorAsExpected(expectedError);
}


int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_canm_init_normal_init);
    UNITY_END();

    return 0;
}