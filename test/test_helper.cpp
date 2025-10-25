#include "test_helper.h"
#include <stdio.h>
#include <stdarg.h>

static tERRH_ERRORDATA_STR lastErrorCode;
tERRH_ERRORDATA_STR noErrorCode =
{
    .moduleId = 0,
    .instanceId = 0,
    .apiId = 0,
    .errorId = 0,
    .errorLvl = ERRH_ERRORTYPE_UNDEF,
    .ti_globalTime = {
        .year_U16 = 0,
        .month_U8 = 0,
        .day_U8 = 0,
        .hour_U8 = 0,
        .minute_U8 = 0,
        .second_U8 = 0,
    },
    .ti_us_timestamp = 0,
    .s_sentOnCan_U8 = 0,
    .count_U8 = 0,
};

static int64_t (helper_ti_us_readSystemTimeCallback_S64)(void)
{
    static int64_t time = 0;
    return time++;
}

static tVARS_TIMEDATA_STR (helper_ti_readCurrentGlobalTimeCallback_S64)(void)
{
    static tVARS_TIMEDATA_STR timedate = {
        .year_U16 = 0,
        .month_U8 = 0,
        .day_U8 = 0,
        .hour_U8 = 0,
        .minute_U8 = 0,
        .second_U8 = 0,
    };
    timedate.second_U8++;
    return timedate;
}

static void testErrhCallback(tERRH_ERRORTYPE_E ErrorLvl, uint32_t ModuleId, uint32_t InstanceId, uint32_t ApiId, uint32_t ErrorId)
{
    lastErrorCode.moduleId = ModuleId;
    lastErrorCode.instanceId = InstanceId;
    lastErrorCode.apiId = ApiId;
    lastErrorCode.errorId = ErrorId;
    lastErrorCode.errorLvl = ErrorLvl;
    lastErrorCode.ti_globalTime = helper_ti_readCurrentGlobalTimeCallback_S64();
    lastErrorCode.ti_us_timestamp = helper_ti_us_readSystemTimeCallback_S64();
    lastErrorCode.s_sentOnCan_U8 = 1;
    lastErrorCode.count_U8 = 1;
}

void test_errh_setUp(void)
{
    lastErrorCode = {0};
    static tERRH_INITDATA_STR errhInit =
    {
        .ti_us_readSystemTimeCallback_S64 = helper_ti_us_readSystemTimeCallback_S64,
        .ti_readCurrentGlobalTimeCallback_S64 = helper_ti_readCurrentGlobalTimeCallback_S64,
        .reportErrorCallback = testErrhCallback,
    };
    errh_init(&errhInit);
}

void test_errh_tearDown(void)
{
    lastErrorCode.moduleId = 0;
    lastErrorCode.instanceId = 0;
    lastErrorCode.apiId = 0;
    lastErrorCode.errorId = 0;
    lastErrorCode.errorLvl = ERRH_ERRORTYPE_UNDEF;
    lastErrorCode.count_U8 = 0;
}

void test_errh_isLastErrorAsExpected(tERRH_ERRORDATA_STR expected)
{
    tB asExpected = true;
    char text[256] = {0};
    tU8 textPos = 0;

    if (expected.moduleId != lastErrorCode.moduleId)
    {
        asExpected = false;
        textPos += sprintf(&text[textPos], "Module ID expected %u, was %u | ", expected.moduleId, lastErrorCode.moduleId);
    }
    if (expected.instanceId != lastErrorCode.instanceId)
    {
        asExpected = false;
        textPos += sprintf(&text[textPos], "Instance ID expected %u, was %u | ", expected.instanceId, lastErrorCode.instanceId);
    }
    if (expected.apiId != lastErrorCode.apiId)
    {
        asExpected = false;
        textPos += sprintf(&text[textPos], "API ID expected %u, was %u | ", expected.apiId, lastErrorCode.apiId);
    }
    if (expected.errorId != lastErrorCode.errorId)
    {
        asExpected = false;
        textPos += sprintf(&text[textPos], "Error ID expected %u, was %u | ", expected.errorId, lastErrorCode.errorId);
    }
    if (expected.errorLvl != lastErrorCode.errorLvl)
    {
        asExpected = false;
        textPos += sprintf(&text[textPos], "Error Level expected %u, was %u | ", expected.errorLvl, lastErrorCode.errorLvl);
    }

    UNITY_TEST_ASSERT(asExpected, __LINE__, text);
}

char* errorline(const char* text_form, ...) {
    static char buffer[1024];
    va_list args;
    va_start(args, text_form);
    vsnprintf(buffer, sizeof(buffer), text_form, args);
    va_end(args);
    return buffer;
}

tU32 extract_u32_from_bytes(const tU8* data)
{
    return ((tU32)data[0]) |
           ((tU32)data[1] << 8) |
           ((tU32)data[2] << 16) |
           ((tU32)data[3] << 24);
}

tB rtdb_lock_callback(void)
{
    return true;
}

void rtdb_unlock_callback(void)
{
    return;
}

tU32 (CRC32_callback)(const tU8* data_U8, tU32 length_U32)
{
    return 0;
}

void setUp(void) {
    test_errh_setUp();
    tRTDB_INITDATA_STR rtdbInit =
    {
        .rtdb_lock_callback = rtdb_lock_callback,
        .rtdb_unlock_callback = rtdb_unlock_callback,
        .CRC32_callback = CRC32_callback,
    };
    rtdb_init(&rtdbInit);
    canm_rtdb_init();
    dimh_rtdb_init();
    diom_rtdb_init();
    mbcm_rtdb_init();
}

void tearDown(void) {
    test_errh_tearDown();
}