#include "test_helper.h"
#include <unity.h>
#include "rtdb.h"
#include "mdll.h"

void test_rtdb_dummy(void)
{

}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_rtdb_dummy);
    UNITY_END();

    return 0;
}