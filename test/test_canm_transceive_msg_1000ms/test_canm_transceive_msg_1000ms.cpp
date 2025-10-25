#include "test_helper.h"
#include <unity.h>
#include "canm.h"
#include "rtdb.h"
#include "mdll.h"
#include <vector>
#include <cstdio>

using namespace std;

#define TEST_ti_ms_TOTAL_EXECUTION_TIME_U32 ((tU32) 5000000)

static tU32 totalMsgsSent = 0;
static vector<tCANM_MSG> messages = {};

static tB send(const tCANM_MSG* msg)
{
    messages.push_back(*msg);
    totalMsgsSent++;
    return NO_ERROR;
}

static tB receive(tCANM_MSG* msg) { return NO_ERROR; }

static tS32 find_position_in_vector_for_message(tU32 msgId)
{
    for (size_t i = 0; i < messages.size(); ++i)
    {
        if (messages[i].identifier == msgId)
            return static_cast<tS32>(i);
    }
    return -1;
}

static tB message_should_be_present_at_time(tU32 time)
{
    return (time > 0) && (time % 1000 == 0);
}

static void update_and_write_relay_data(tU8 startEnum, tU8 endEnum)
{
    static tU8 relayData[16] = {0};

    for (tU8 relayBoard = startEnum; relayBoard <= endEnum; ++relayBoard)
    {
        tU8 index = relayBoard - startEnum;
        relayData[index] ^= relayBoard + (relayData[index] << 1) + 0x1F;
        rtdb_write_tU8S((tU8SEnumT)relayBoard, relayData[index]);
    }
}

static void prepare_data_for_message(tU32 id)
{
    tU32 moduleIndx = (id - 0x1F5) / 11;
    tU32 msgIndx = (id - 0x1F5) % 11;

    tF32 currVal1 = rtdb_read_tF32S((tF32SEnumT)(RTDB_MBCM_X_SDM120MREADINGS_0_AF32_0 + (msgIndx * 2) + 0 + moduleIndx * 21));
    currVal1 += id + moduleIndx + msgIndx;
    rtdb_write_tF32S((tF32SEnumT)(RTDB_MBCM_X_SDM120MREADINGS_0_AF32_0 + (msgIndx * 2) + 0 + moduleIndx * 21), currVal1);
    if (msgIndx < 10)
    {
        tF32 currVal2 = rtdb_read_tF32S((tF32SEnumT)(RTDB_MBCM_X_SDM120MREADINGS_0_AF32_0 + (msgIndx * 2) + 1 + moduleIndx * 21));
        currVal2 += id + moduleIndx + msgIndx;
        rtdb_write_tF32S((tF32SEnumT)(RTDB_MBCM_X_SDM120MREADINGS_0_AF32_0 + (msgIndx * 2) + 1 + moduleIndx * 21),currVal2);
    }
}

static tF32 reencode_first_var_from_message_to_float(tU32 id)
{
    tS32 msgPos = find_position_in_vector_for_message(id);
    tU32 raw = (messages[msgPos].data[0] << 0) | (messages[msgPos].data[1] << 8) | (messages[msgPos].data[2] << 16) | (messages[msgPos].data[3] << 24);
    tF32 reencoded = *((tF32*)((void*)(&raw)));
    return reencoded;
}

static tF32 reencode_second_var_from_message_to_float(tU32 id)
{
    tS32 msgPos = find_position_in_vector_for_message(id);
    tU32 raw = (messages[msgPos].data[4] << 0) | (messages[msgPos].data[5] << 8) | (messages[msgPos].data[6] << 16) | (messages[msgPos].data[7] << 24);
    return *((tF32*)((void*)(&raw)));
}

static void verify_content_of_message(tU32 id)
{
    tU32 moduleIndx = (id - 0x1F5) / 11;
    tU32 msgIndx = (id - 0x1F5) % 11;

    tF32 expected1 = rtdb_read_tF32S((tF32SEnumT)(RTDB_MBCM_X_SDM120MREADINGS_0_AF32_0 + (msgIndx * 2) + 0 + moduleIndx * 21));
    tF32 actual1 = reencode_first_var_from_message_to_float(id);
    UNITY_TEST_ASSERT_EQUAL_FLOAT(expected1, actual1, __LINE__, "Message var 1 was not decoded correctly!");

    if (msgIndx < 10)
    {
        tF32 expected1 = rtdb_read_tF32S((tF32SEnumT)(RTDB_MBCM_X_SDM120MREADINGS_0_AF32_0 + (msgIndx * 2) + 1 + moduleIndx * 21));
        tF32 actual1 = reencode_second_var_from_message_to_float(id);
        UNITY_TEST_ASSERT_EQUAL_FLOAT(expected1, actual1, __LINE__, "Message var 2 was not decoded correctly!");
    }
}

static void verify_message_with_id_at_time(tU32 id, tU32 time)
{
    tS32 msgPos = find_position_in_vector_for_message(id);
    if (message_should_be_present_at_time(time))
    {
        UNITY_TEST_ASSERT(msgPos >= 0, __LINE__, errorline("Message 0x%x was not present at time %d!", id, time));
        verify_content_of_message(id);
    }
    else
    {
        UNITY_TEST_ASSERT_EQUAL_INT32(-1, msgPos, __LINE__, errorline("Message 0x%x was present at time %d!", id, time));
    }
}

static void check_1000ms_msg()
{
    tU32 ti_ms_executionTime_U32 = 0;

    while (ti_ms_executionTime_U32 <= TEST_ti_ms_TOTAL_EXECUTION_TIME_U32)
    {
        ti_ms_executionTime_U32 += TEST_ti_ms_MAIN_FUNC_CYCLE_TIME_U32;
        static tU32 moduleIndx = 0;

        if (ti_ms_executionTime_U32 % 1000 == 0)
        {
            for (tU32 id = 0x1F5 + moduleIndx * 11; id < (0x1F5 + moduleIndx * 11 + 11); id++)
            {
                prepare_data_for_message(id);
            }
        }

        canm_transceive_run_5ms();

        for (tU32 id = 0x1F5 + moduleIndx * 11; id < (0x1F5 + moduleIndx * 11 + 11); id++)
        {
            verify_message_with_id_at_time(id, ti_ms_executionTime_U32);
        }

        if (ti_ms_executionTime_U32 % 1000 == 0)
        {
            moduleIndx++;
            if (moduleIndx >= CANM_MAX_PWR_METERS_U32)
            {
                moduleIndx = 0;
            }
        }

        messages.clear();
    }

    test_errh_isLastErrorAsExpected(noErrorCode);
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();

    tCANM_INITDATA_STR canmInitCfg_str = {
        .ti_ms_taskDelay_U32 = TEST_ti_ms_MAIN_FUNC_CYCLE_TIME_U32,
        .canm_send_callback = send,
        .canm_receive_callback = receive,
    };

    canm_init(&canmInitCfg_str);

    RUN_TEST(check_1000ms_msg);

    UNITY_END();
    return 0;
}