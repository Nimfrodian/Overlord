/*
 Copyright (c) 2014-present PlatformIO <contact@platformio.org>

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
**/

#include <unity.h>
#include "RelayControl.h"

void setUp(void) {
    RelayControl_init();
}

void tearDown(void) {
    // clean stuff up here
}

void test_RelayControl_composeCanMsg(void)
{
    uint8_t msgData[8] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
    uint32_t canId = 0xA5;

    RelayControl_composeCanMessage(msgData, &canId);

    TEST_ASSERT_EQUAL(0x110, canId);
    uint8_t expected[8] = {0};
    TEST_ASSERT_EQUAL_INT8_ARRAY(expected, msgData, 8);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_RelayControl_composeCanMsg);
    UNITY_END();

    return 0;
}
