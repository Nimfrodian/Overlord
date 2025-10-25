#include "test_helper.h"
#include <unity.h>
#include "canm.h"
#include "rtdb.h"
#include "mdll.h"
#include <vector>
#include <cstdio>

using namespace std;

#define TEST_ti_ms_TOTAL_EXECUTION_TIME_U32 ((tU32)5000000)

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
    return (time > 0) && (time % 100 == 0);
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
    switch (id)
    {
        case 0x110:
            update_and_write_relay_data(
                (tU8)RTDB_MBCM_X_ACTUALRELAYSTATES_AU8_0,
                (tU8)RTDB_MBCM_X_ACTUALRELAYSTATES_AU8_7
            );
            break;

        case 0x111:
            update_and_write_relay_data(
                (tU8)RTDB_MBCM_X_ACTUALRELAYSTATES_AU8_8,
                (tU8)RTDB_MBCM_X_ACTUALRELAYSTATES_AU8_15
            );
            break;

        case 0x150:
        {
            static tU32 gpioData = 0xA5A5A5A5;
            gpioData ^= (gpioData << 1) + 0x1F3;
            rtdb_write_tU32S(RTDB_DIOM_X_INPUTSTATES_U32, gpioData);
            break;
        }
    }
}

static void verify_relay_message(tU32 id, tU8 startEnum)
{
    static tU8 relayData[8] = {0};

    for (tU8 i = 0; i < 8; ++i)
    {
        tU8 relayBoard = startEnum + i;
        relayData[i] ^= relayBoard + (relayData[i] << 1) + 0x1F;
    }

    tS32 msgPos = find_position_in_vector_for_message(id);
    UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(relayData, messages[msgPos].data, 8, __LINE__,
        errorline("Message was not reconstructed properly for 0x%x!", id));
}

static void verify_content_of_message(tU32 id)
{
    switch (id)
    {
        case 0x150:
        {
            static tU32 gpioData = 0xA5A5A5A5;
            gpioData ^= (gpioData << 1) + 0x1F3;
            tS32 msgPos = find_position_in_vector_for_message(0x150);
            tU32 reconstructed = extract_u32_from_bytes(messages[msgPos].data);
            UNITY_TEST_ASSERT_EQUAL_UINT32(gpioData, reconstructed, __LINE__,
                errorline("Message was not reconstructed properly for 0x150!"));
            break;
        }

        case 0x110:
            verify_relay_message(0x110, (tU8)RTDB_MBCM_X_ACTUALRELAYSTATES_AU8_0);
            break;

        case 0x111:
            verify_relay_message(0x111, (tU8)RTDB_MBCM_X_ACTUALRELAYSTATES_AU8_8);
            break;
    }
}

static void verify_message_with_id_at_time(tU32 id, tU32 time)
{
    tS32 msgPos = find_position_in_vector_for_message(id);
    if (message_should_be_present_at_time(time))
    {
        UNITY_TEST_ASSERT(msgPos >= 0, __LINE__,
            errorline("Message 0x%x was not present at time %d!", id, time));
        verify_content_of_message(id);
    }
    else
    {
        UNITY_TEST_ASSERT_EQUAL_INT32(-1, msgPos, __LINE__,
            errorline("Message 0x%x was present at time %d!", id, time));
    }
}

static void check_100ms_msg()
{
    tU32 ti_ms_executionTime_U32 = 0;

    while (ti_ms_executionTime_U32 <= TEST_ti_ms_TOTAL_EXECUTION_TIME_U32)
    {
        ti_ms_executionTime_U32 += TEST_ti_ms_MAIN_FUNC_CYCLE_TIME_U32;

        if (ti_ms_executionTime_U32 % 100 == 0)
        {
            prepare_data_for_message(0x110);
            prepare_data_for_message(0x111);
            prepare_data_for_message(0x150);
        }

        canm_transceive_run_5ms();

        verify_message_with_id_at_time(0x110, ti_ms_executionTime_U32);
        verify_message_with_id_at_time(0x111, ti_ms_executionTime_U32);
        verify_message_with_id_at_time(0x150, ti_ms_executionTime_U32);

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

    RUN_TEST(check_100ms_msg);

    UNITY_END();
    return 0;
}