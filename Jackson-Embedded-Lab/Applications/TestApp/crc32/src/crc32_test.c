#include "crc32.h"

#define MAX_PATH_CACHE_LENGTH 0xffffu

uint8_t data_cache[DATA_POOL_MAX_SIZE];
// uint8_t crc_register[CRC_32_REGISHTER_LENGTH];

void vdCmdInputCRCCalculate()
{
    size_t len = 0;
    uint32_t CRC = 0;
    while(1)
    {
        memset(data_cache, 0x00, DATA_POOL_MAX_SIZE);
        printf("Please input the test data: \n");
        printf("\n");
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
        printf("\n");
    }
    return;
}

void vdFileInputCRCCalculate()
{
    uint8_t path_cache[MAX_PATH_CACHE_LENGTH];
    size_t path_len = 0;
    FILE *fp;
    size_t read_size = 0;
    size_t total_size = 0;
    uint32_t CRC = 0;
    
    while(1)
    {
        path_len  = 0;
        read_size = 0;
        memset(path_cache, 0x00, MAX_PATH_CACHE_LENGTH);
        printf("Please input file path: \n");
        if(fgets(path_cache, MAX_PATH_CACHE_LENGTH, stdin) == NULL)
        {
            continue;
        }
        path_len = strlen(path_cache);
        if (path_len > 0 && path_cache[path_len-1] == '\n')
        {
            path_cache[path_len-1] = '\0';
            path_len--;
        }
        if(0 == strcmp(path_cache, "exit"))
        {
            break;
        }

        fp = fopen(path_cache, "rb");
        if(NULL == fp)
        {
            printf("open path failed: %s\n", path_cache);
            continue;
        }

        memset(data_cache, 0x00, DATA_POOL_MAX_SIZE);
        u32CrcIeee8023_Init(&CRC);

        while((read_size = fread(data_cache, sizeof(uint8_t), DATA_POOL_MAX_SIZE, fp)) > 0)
        {
            u32CrcIeee8023_Excu(&CRC, data_cache, read_size);
            total_size += read_size;
        }
        if(0 == read_size)
        {
            u32CrcIeee8023_Final(&CRC);
        }
        fclose(fp);
        printf("CRC in file is: %08X\n", CRC);
    }
    return;

}

int main()
{
    char choice[8];

    while (1)
    {
        printf("=== CRC Calculator ===\n");
        printf("1. Input string from command line\n");
        printf("2. Input file path and calculate CRC\n");
        printf("q. Quit\n");
        printf("Please select: ");

        memset(choice, 0x00, sizeof(choice));
        if (fgets(choice, sizeof(choice), stdin) == NULL)
        {
            continue;
        }

        if (choice[0] == '1')
        {
            vdCmdInputCRCCalculate();
        }
        else if (choice[0] == '2')
        {
            vdFileInputCRCCalculate();
        }
        else if (choice[0] == 'q' || choice[0] == 'Q')
        {
            break;
        }
        else
        {
            printf("Invalid choice, please try again.\n\n");
        }
    }
    /* 
            data_cache: 0x01, len: 4
            CRC: 0515289C
            123456789
            data_cache: 123456789, len: 9
            CRC: CBF43926    
    */
    return 0;
}