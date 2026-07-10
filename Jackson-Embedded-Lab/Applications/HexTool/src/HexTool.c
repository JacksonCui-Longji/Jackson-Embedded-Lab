#include "HexTool.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "CommonType.h"

#define READ_LENGTH_MAX 0x08u
#define DISPLAY_LENGTH_MAX ((READ_LENGTH_MAX*3)+1)

static uint8_t ChrToHex(uint8_t Chr)
{
    uint8_t tmp_byte = RET_NG;

    if(('0' <= Chr) && (Chr <= '9'))
    {
        tmp_byte = Chr -'0';
    }
    else if(('A' <= Chr) && (Chr <= 'F'))
    {
        tmp_byte = Chr - 'A';
    }
    else if(('a' <= Chr) && (Chr <= 'f'))
    {
        tmp_byte = Chr - 'a';
    }
    else
    {
        tmp_byte = RET_NG;
    }
    return tmp_byte;
}

static void u8HexToolWrite_Init()
{
    //
}

static size_t u8HexToolWrite_Update(uint8_t *inData, size_t len, uint8_t *output_cache, size_t outlen)
{
    uint32_t index = 0;
    size_t output_cache_len = 0;

    memset(output_cache, 0x00, outlen);
    if(0 != len%2)
    {
        // if input string length is singular, add 0 at position 0 to fit hex
        output_cache[output_cache_len] = '0';
        output_cache_len++;
    }

    while((index < len) && (output_cache_len <= outlen))
    {
        uint8_t tmp = ChrToHex(inData[index]);
        if(RET_NG == tmp)
        {
            printf("Invalid input: %s, at index: %d skip it!\n", inData, index);
        }
        else
        {
            output_cache[output_cache_len] = tmp;
            output_cache_len++;
        }
        index++;
    }
    return output_cache_len;
}

static uint8_t u8HexToolWrite_Final(uint8_t *inData, size_t len)
{
    FILE *fp;
    size_t index = 0;
    uint8_t final_cache[MAX_INPUT_STRING_LENGTH/2+1] = {0};
    size_t final_cache_len = 0;

    while(index < len)
    {
        final_cache[final_cache_len] = (inData[index] << 4) | (inData[index+1]);
        final_cache_len++;

        index+=2;
    } 

    fp = fopen("output.bin", "wb");
    if(NULL == fp)
    {
        perror("failed open/create failed!");
        return RET_NG;
    }
    fwrite(final_cache, 1, final_cache_len, fp);
    fclose(fp);
    return RET_OK;
}

uint8_t u8HexToolWrite(uint8_t *inData, size_t len)
{
    uint8_t tmp_cache[MAX_INPUT_STRING_LENGTH] = {0};
    size_t tmp_cache_len= 0;
    
    
    u8HexToolWrite_Init();
    
    tmp_cache_len = u8HexToolWrite_Update(inData, len, tmp_cache, MAX_INPUT_STRING_LENGTH);
    if(0 >= tmp_cache_len)
    {
        printf("update Chr to Hex cache failed!\n");
        return RET_NG;
    }

    return u8HexToolWrite_Final(tmp_cache, tmp_cache_len);

}

uint8_t u8HexToolRead(uint8_t *inPath, size_t len)
{
    uint8_t actual_len = 0;
    uint32_t total_len = 0;
    uint8_t read_cache[READ_LENGTH_MAX+1] = {0};
    uint8_t ascii_cache[READ_LENGTH_MAX+1] = {0};
    FILE *fp;

    uint32_t display_offset = 0x00000000u;
    uint8_t display_cache[DISPLAY_LENGTH_MAX] = {0};


    printf("u8HexToolRead in \n");
    fp = fopen(inPath, "rb");
    if(NULL == fp)
    {
        printf("fopen failed!\n");
        return RET_NG;
    }

    while ((actual_len = fread(read_cache, 1, READ_LENGTH_MAX, fp)) > 0)
    {
        uint8_t i_rc = 0;
        uint8_t i_dc = 0;

        read_cache[actual_len] = '\0';

        memset(display_cache, 0x00, DISPLAY_LENGTH_MAX);
        memset(ascii_cache, 0x00, READ_LENGTH_MAX+1);
        while(i_rc < actual_len)
        {
            // this part is to transfer Hex to three characters, not a normal HEX to Char, and only support uppercase.
            uint8_t hi = read_cache[i_rc] / 16;
            uint8_t lo = read_cache[i_rc] % 16;
            display_cache[i_dc]   = (hi < 10) ? (hi + '0') : (hi - 10 + 'A');
            display_cache[i_dc+1] = (lo < 10) ? (lo + '0') : (lo - 10 + 'A');
            display_cache[i_dc+2] = ' ';

            if((read_cache[i_rc] >=  0x20u) && (read_cache[i_rc] <= 0x7eu))
            {
                ascii_cache[i_rc] = read_cache[i_rc];
            }
            else
            {
                ascii_cache[i_rc] = '-';
            }
            i_dc = i_dc+3;
            i_rc++;
        }

        printf("%08X\n", display_offset);
        printf("%s\n", display_cache);
        printf("%s\n\n", ascii_cache);
        total_len += actual_len;
        display_offset = total_len;
    }

    printf("read finish! Total length: %d\n", total_len);

    fclose(fp);
    return 1;
}
