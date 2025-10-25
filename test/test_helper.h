#pragma once
#include <unity.h>
#include "errh.h"
#include "rtdb.h"

#define TEST_ti_ms_MAIN_FUNC_CYCLE_TIME_U32 ((tU32) 5)

extern tERRH_ERRORDATA_STR noErrorCode;

void test_errh_setUp(void);
void test_errh_tearDown(void);
tERRH_ERRORDATA_STR test_errh_getLastError(void);
void test_errh_isLastErrorAsExpected(tERRH_ERRORDATA_STR expected);
char* errorline(const char* text_form, ...);
tU32 extract_u32_from_bytes(const tU8* data);