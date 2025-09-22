#pragma once
#include <unity.h>
#include "errh.h"
#include "rtdb.h"

#define TEST_ti_ms_EXECUTION_TIME_U32 ((tU32) 5)

extern tERRH_ERRORDATA_STR noErrorCode;

void test_errh_setUp(void);
void test_errh_tearDown(void);
tERRH_ERRORDATA_STR test_errh_getLastError(void);
void test_errh_isLastErrorAsExpected(tERRH_ERRORDATA_STR expected);