#include "string.h"
#include "stdint.h"
#include "stdio.h"

#define CRC32_POLYNOMIAL 0x04C11DB7u
#define CRC_REGISTER_INIT 0xffffffffu
#define DATA_POOL_MAX_SIZE 0xffffu

#define CRC_32_REGISHTER_LENGTH 4

uint8_t data_cache[DATA_POOL_MAX_SIZE];
uint8_t crc_register[CRC_32_REGISHTER_LENGTH];

void vdInitCrc()
{
    // init crc register
    memset(crc_register, 0xff, CRC_32_REGISHTER_LENGTH);
}

uint8_t u8ReverseBIT_8(uint8_t inData)
{
    inData = ((inData >> 1)&0x55) | ((inData&0x55) << 1);
    inData = ((inData >> 2)&0x33) | ((inData&0x33) << 2);
    inData = ((inData >> 4)&0x0f) | ((inData&0x0f) << 4);
    return inData;
}

uint32_t u8ReverseBIT_32(uint32_t inData)
{
    inData = ((inData >> 1)&0x55555555u) | ((inData&0x55555555u) << 1);
    inData = ((inData >> 2)&0x33333333u) | ((inData&0x33333333u) << 2);
    inData = ((inData >> 4)&0x0f0f0f0fu) | ((inData&0x0f0f0f0fu) << 4);
    inData = ((inData >> 8)&0x00ff00ffu) | ((inData&0x00ff00ffu) << 8);
    inData = ((inData >> 16)&0x0000ffffu) | ((inData&0x0000ffffu) << 16);
    return inData;
}

uint32_t u32CrcIeee8023(uint8_t* inData, size_t len)
{
    uint32_t retCrc = 0xffffffff;
    for(int index = 0; index < len; index++)
    {
        inData[index] = u8ReverseBIT_8(inData[index]);
        retCrc ^= (inData[index] << 24);

        for(int j = 0; j < 8 ; j++)
        {
            if(retCrc & 0x80000000u)
            {
                retCrc = (retCrc << 1) ^ CRC32_POLYNOMIAL;
            }
            else
            {
                retCrc = (retCrc << 1);
            }
        }
    }

    retCrc = u8ReverseBIT_32(retCrc);
    retCrc ^= 0xffffffffu;

    return retCrc;
}

int main()
{
    size_t len = 0;
    uint32_t CRC = 0;
    memset(data_cache, 0x00, DATA_POOL_MAX_SIZE);
    while(1)
    {
        if(fgets(data_cache, DATA_POOL_MAX_SIZE, stdin) != NULL)
        {
            len = strlen(data_cache)-1;
            data_cache[len] = '\0';
            if(0 == strcmp(data_cache, "exit"))
            {
                break;
            }
            printf("data_cache: %s, len: %ld\n", data_cache, len);

            CRC = u32CrcIeee8023(data_cache, len);

            printf("CRC: %08X\n", CRC);
            memset(data_cache, 0x00, DATA_POOL_MAX_SIZE);
        }
    }
    /* 
            data_cache: 0x01, len: 4
            CRC: 0515289C
            123456789
            data_cache: 123456789, len: 9
            CRC: CBF43926    
    */
}