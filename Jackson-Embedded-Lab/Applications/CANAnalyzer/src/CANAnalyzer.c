#include "CANAnalyzer.h"
#include "stdlib.h"
#include "string.h"

#define FILE_READ_BUFFER_LENGTH 0x400u

static uint8_t* spaceignore(uint8_t *ptr)
{
    if(NULL != ptr)
    {
        while((' ' == *ptr) || ('\t' == *ptr))
        {
            ptr++;
        }
    }
    else
    {
        // NULL ptr, do nothing.
    }
    return ptr;
}


void vdCANAnalyzeFile(uint8_t *path, size_t path_len)
{

    if((NULL == path) || (0 == path_len))
    {
        printf("Please input valid path!\n");
        return;
    }

    FILE *fp = NULL;
    fp = fopen(path, "r");
    if(NULL == fp)
    {
        printf("open path %s failed!\n", path);
        return;
    }

    uint8_t readbuffer[FILE_READ_BUFFER_LENGTH] = {0};
    while(fgets(readbuffer, sizeof(FILE_READ_BUFFER_LENGTH), fp))
    {
        vdCanFrameAnalyze(readbuffer, strlen(readbuffer));
    }

    fclose(fp);
    return;
}

void vdCanFrameAnalyze(uint8_t *frame, size_t len)
{
    uint8_t *cursor = frame;
    if((NULL == frame) ||  (0 == len))
    {
        printf("Please input valid frame to analyze!\n");
        return;
    }

    cursor = spaceignore(cursor);

    if(('\n' == *cursor) || ('\r' == *cursor) || ('\0' == *cursor))
    {
        printf("Empty line!\n");
        return;
    }

    if((('/' == *cursor) && ('/' == *(cursor+1))) || ('#' == *cursor))
    {
        printf("ignore comment.\n");
        return;
    }

    char *endptr = NULL;
    double timestamp = strtod(cursor, &endptr);

    if(endptr == cursor)
    {
        printf("not can trace, ignore!\n");
        return;
    }
    else
    {
        cursor = endptr;
    }

    if((' ' != *cursor) && ('\t' != *cursor))
    {
        printf("format error, this buffer is not CAN frame!\n");
        return;
    }



}