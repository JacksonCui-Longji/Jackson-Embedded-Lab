#include "crc32.h"

#define CRC32_POLYNOMIAL 0x04C11DB7u
#define CRC_REGISTER_INIT 0xffffffffu

static uint8_t u8ReverseBIT_8(uint8_t inData)
{
    inData = ((inData >> 1)&0x55) | ((inData&0x55) << 1);
    inData = ((inData >> 2)&0x33) | ((inData&0x33) << 2);
    inData = ((inData >> 4)&0x0f) | ((inData&0x0f) << 4);
    return inData;
}

static uint32_t u8ReverseBIT_32(uint32_t inData)
{
    inData = ((inData >> 1)&0x55555555u) | ((inData&0x55555555u) << 1);
    inData = ((inData >> 2)&0x33333333u) | ((inData&0x33333333u) << 2);
    inData = ((inData >> 4)&0x0f0f0f0fu) | ((inData&0x0f0f0f0fu) << 4);
    inData = ((inData >> 8)&0x00ff00ffu) | ((inData&0x00ff00ffu) << 8);
    inData = ((inData >> 16)&0x0000ffffu) | ((inData&0x0000ffffu) << 16);
    return inData;
}

void u32CrcIeee8023_Init(uint32_t *crc)
{
    // init crc register
    if(NULL != crc)
    {
        *crc = CRC_REGISTER_INIT;
    }
    return;
}

void u32CrcIeee8023_Excu(uint32_t *retCrc, uint8_t* inData, size_t len)
{

    if(NULL == retCrc)
    {
        printf("retCrc error!\n");
        return;
    }

    if((NULL == inData) || (0 == len))
    {
        printf("inData or len error!\n");
        return;
    }
    
    for(uint32_t index = 0; index < len; index++)
    {
        inData[index] = u8ReverseBIT_8(inData[index]);
        *retCrc ^= (inData[index] << 24);

        for(uint8_t j = 0; j < 8 ; j++)
        {
            if(*retCrc & 0x80000000u)
            {
                *retCrc = (*retCrc << 1) ^ CRC32_POLYNOMIAL;
            }
            else
            {
                *retCrc = (*retCrc << 1);
            }
        }
    }
}

void u32CrcIeee8023_Final(uint32_t *retCrc)
{
    if(NULL == retCrc)
    {
        printf("retCrc error!\n");
        return;
    }

    *retCrc = u8ReverseBIT_32(*retCrc);
    *retCrc ^= 0xffffffffu;
}

uint32_t u32CrcIeee8023(uint8_t* inData, size_t len)
{
    uint32_t retCrc = 0;

    if((NULL == inData) || (0 == len))
    {
        printf("inData or len error!\n");
        return 0;
    }

    u32CrcIeee8023_Init(&retCrc);
    u32CrcIeee8023_Excu(&retCrc, inData, len);
    u32CrcIeee8023_Final(&retCrc);

    return retCrc;
}
