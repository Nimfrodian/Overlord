#pragma once
#include <stdint.h>

#define TU8_MIN (0)
#define TU8_MAX (UCHAR_MAX)
#define TU16_MIN (0)
#define TU16_MAX (USHRT_MAX)
#define TU32_MIN (0)
#define TU32_MAX (UINT_MAX)
#define TS8_MIN (SCHAR_MIN)
#define TS8_MAX (SCHAR_MAX)
#define TS16_MIN (SHRT_MIN)
#define TS16_MAX (SHRT_MAX)
#define TS32_MIN (INT_MIN)
#define TS32_MAX (INT_MAX)
#define TB_MIN (0)
#define TB_MAX (1)
#define TE_MIN (0)
#define TE_MAX (TU32_MAX)
#define TF32_MIN (-FLT_MAX)
#define TF32_MAX (FLT_MAX)

typedef uint8_t tU8;
typedef uint16_t tU16;
typedef uint32_t tU32;
typedef int8_t tS8;
typedef int16_t tS16;
typedef int32_t tS32;
typedef tU8 tB;
typedef tU32 tE;
typedef float tF32;