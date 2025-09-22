#include "test_helper.h"
#include <unity.h>
#include "canm.h"
#include "mdll.h"

void test_canm_init_null_ptr(void)
{
    canm_init(nullptr);

    tERRH_ERRORDATA_STR expectedError =
    {
        .moduleId = MODULE_CANM,
        .instanceId = 0,
        .apiId = CANM_API_INIT_U32,
        .errorId = ERRH_POINTER_IS_NULL,
        .errorLvl = ERRH_ERROR_CRITICAL,
    };
    test_errh_isLastErrorAsExpected(expectedError);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_canm_init_null_ptr);
    UNITY_END();

    return 0;
}