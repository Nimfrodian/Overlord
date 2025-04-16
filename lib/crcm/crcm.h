/*
*   CRC calculation module
*
*
*/

#pragma once
#include "vars.h"

/**
 * @brief CRC16 calculation function
 * @param str message content on which CRC is to be calculated
 * @param usDataLen length of the message (excluding CRC bytes)
 * @return CRC16 value
 */
tU16 crcm_CRC16_Modbus(tU8* str, tU32 usDataLen);

/**
 * @brief CRC32 calculation function
 * @param data_U8 data to be CRC'd
 * @param length_U32 length of the data
 * @return CRC32 value
 */
tU32 crcm_CRC32(const tU8* data_U8, tU32 length_U32);