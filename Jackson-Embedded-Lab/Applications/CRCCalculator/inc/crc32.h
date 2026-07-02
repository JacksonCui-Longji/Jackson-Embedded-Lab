#ifndef _CRC32_H_
#define _CRC32_H_

#include "string.h"
#include "stdint.h"
#include "stdio.h"

#define DATA_POOL_MAX_SIZE 0xffffu
#define CRC_32_REGISHTER_LENGTH 4

extern void u32CrcIeee8023_Init(uint32_t *crc);
extern void u32CrcIeee8023_Excu(uint32_t *retCrc, uint8_t* inData, size_t len);
extern void u32CrcIeee8023_Final(uint32_t *retCrc);

extern uint32_t u32CrcIeee8023(uint8_t* inData, size_t len);

#endif // _CRC32_H_