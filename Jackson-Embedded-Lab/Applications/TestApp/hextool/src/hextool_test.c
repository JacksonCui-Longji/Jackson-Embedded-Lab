#include "HexTool.h"
#include "stdio.h"
#include "unistd.h"
#include "string.h"

#define MAX_PATH_CACHE_LENGTH 0xffffu
void vdFileToHex()
{
    uint8_t path_cache[MAX_PATH_CACHE_LENGTH];
    size_t path_len = 0;
    uint8_t ret = 0;

    while(1)
    {
        path_len  = 0;
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
        ret = u8HexToolRead(path_cache, path_len);
    }
}

void vdHexToFile()
{
    FILE *fp;
    uint8_t input_cache[MAX_INPUT_STRING_LENGTH] = {0};
    uint32_t act_len = 0;
    uint8_t ret = 0;

    printf("Please input file content(HEX Mode): \n");
    
    if(NULL == fgets(input_cache, MAX_INPUT_STRING_LENGTH, stdin))
    {
        printf("input fgets failed!\n");
        return;
    }
    
    act_len = strlen(input_cache);
    if((0 == act_len) || ((1 == act_len) && (('\n' == input_cache[0]) || ('\r' == input_cache[0]))))
    {
        printf("nothing input \n");
        return;
    }

    act_len--;
    input_cache[act_len] = '\0';

    // default path is current working path on linux
    ret = u8HexToolWrite(input_cache, act_len);

    return;
}

int main()
{
    char choice[8];

    while (1)
    {
        printf("=== HEX Conversion ===\n");
        printf("1. input file & output hex\n");
        printf("2. input hex & output bin file\n");
        printf("q. Quit\n");
        printf("Please select: ");

        memset(choice, 0x00, sizeof(choice));
        if (fgets(choice, sizeof(choice), stdin) == NULL)
        {
            continue;
        }

        if (choice[0] == '1')
        {
            vdFileToHex();
        }
        else if (choice[0] == '2')
        {
            vdHexToFile();
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
}

/*
int main(int argc, char *argv[])
{

    uint8_t *path_cache = NULL;
    int opt;
    size_t path_len = 0;
    uint8_t ret = 0;

    if(1 > argc)
    {
        printf("Please input the file path after program name! \n");
        return 0;
    }

    while((opt = getopt(argc, argv, ":p:")) != -1) 
    {
        switch(opt) 
        {
            case ':':
                fprintf(stderr, "Please input file path with parameter -p\n");
                break;
            case 'p':
                path_cache = optarg;
                path_len = sizeof(path_cache);
                printf("path: %s, len: %d \n", path_cache, path_len);
                break;
            case '?':
                fprintf(stderr, "Unknown option or missing argument\n");
                return 1;
        }
    }
    
    if (path_cache == NULL)
    {
        fprintf(stderr, "Usage: %s -p <path>\n", argv[0]);
        return 1;
    }


    ret = u8HexToolRead(path_cache, path_len);
    
    return 0;
}
*/
