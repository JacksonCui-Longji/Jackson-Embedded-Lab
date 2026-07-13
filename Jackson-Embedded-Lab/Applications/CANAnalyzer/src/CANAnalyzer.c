#include "CANAnalyzer.h"
#include "stdlib.h"
#include "string.h"
#include "CommonType.h"

static uint8_t CaseFoldTable[256];
static const uint8_t canfd_dlc_to_len[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8,      /* DLC 0~8，for classical can */
    12, 16, 20, 24, 32, 48, 64      /* DLC 9~15，CAN FD专属映射 */
};

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

static uint8_t isCursorMoved(uint8_t *cur_cursor, uint8_t *last_cursor)
{
    if(cur_cursor - last_cursor)
    {
        return RET_OK;
    }
    else
    {
        return RET_NG;
    }
}

static uint8_t isSpace(uint8_t *cur_cursor)
{
    if((' ' == *cur_cursor) || ('\t' == *cur_cursor))
    {
        return RET_OK;
    }
    else
    {
        return RET_NG;
    }
}

// return value: The left length after moving backward
static int isCorrectFormatAndMoveBackward(uint8_t **cursor, uint8_t **last_cursor, int total_len)
{
    int left_len = total_len;
    if((NULL == *cursor) || (NULL == *last_cursor) || (0 >= total_len))
    {
        return left_len;
    }

    if(RET_OK == isCursorMoved(*cursor, *last_cursor))
    {
        left_len = total_len - (*cursor - *last_cursor);
        *last_cursor = *cursor;
    }
    else
    {

        printf("no timestamp, ignore this item!\n");
        return total_len;

    }
    if((' ' == **cursor) || ('\t' == **cursor))
    {
        *cursor = spaceignore(*cursor);
        left_len = left_len - (*cursor - *last_cursor);
        *last_cursor = *cursor;
    }
    else
    {
        // the first character after timestamp must be space or table
        printf("format error, this buffer is not CAN frame!\n");
        return total_len;
    }
    return left_len;
}

void vdCANAnalyzerInit()
{
    for(int i = 0; i < 256; i++)
    {
        if((i >= 'A') && (i <= 'Z'))
        {
            CaseFoldTable[i] = i + ('a' - 'A');
        }
        else
        {
            CaseFoldTable[i] = i;
        }
    }
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

void vdCanFrameAnalyze(uint8_t *frame, int len)
{
    CANAnalyzerInfo can_info = {0};
    // char *endptr = NULL;
    char *cursor = frame;
    char *last_cursor = frame;
    int left_len = len;
    int ret_len = 0;

    if((NULL == frame) ||  (0 == len))
    {
        printf("Please input valid frame to analyze!\n");
        return;
    }

    // ignore started space
    cursor = (char*)spaceignore((uint8_t*)cursor);

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

    // timestamp field
    can_info.timestamp = strtod(last_cursor, &cursor);
    ret_len = isCorrectFormatAndMoveBackward((uint8_t**)&cursor, (uint8_t**)&last_cursor, left_len);
    if((ret_len <= 0) || (ret_len >= left_len))
    {
        printf("move back failed!\n");
        return;
    }
    else
    {
        left_len = ret_len;
    }

    // channel field    
    can_info.channel = strtoul(last_cursor, &cursor, CAN_ANALYZER_DECIMAL);
    ret_len = isCorrectFormatAndMoveBackward((uint8_t**)&cursor, (uint8_t**)&last_cursor, left_len);
    if((ret_len <= 0) || (ret_len >= left_len))
    {
        printf("move back failed!\n");
        return;
    }
    else
    {
        left_len = ret_len;
    }

    // CAN ID
    can_info.canid = strtoul(last_cursor, &cursor, CAN_ANALYZER_HEXADECIMAL);
    ret_len = isCorrectFormatAndMoveBackward((uint8_t**)&cursor, (uint8_t**)&last_cursor, left_len);
    if((ret_len <= 0) || (ret_len >= left_len))
    {
        printf("move back failed!\n");
        return;
    }
    else
    {
        left_len = ret_len;
    }

    // transmission direction
    if(0 == strncmp(cursor, "Rx", CAN_TRANSMISSION_DIRECTION_LEN))
    {
        can_info.direction = CAN_ANALYZER_RX;
        cursor = cursor + CAN_TRANSMISSION_DIRECTION_LEN;
    }
    else if (0 == strncmp(cursor, "Tx", CAN_TRANSMISSION_DIRECTION_LEN))
    {
        can_info.direction = CAN_ANALYZER_TX;
        cursor = cursor + CAN_TRANSMISSION_DIRECTION_LEN;
    }
    else
    {
        printf("can direction field error!\n");
        return;
    }
    ret_len = isCorrectFormatAndMoveBackward((uint8_t**)&cursor, (uint8_t**)&last_cursor, left_len);
    if((ret_len <= 0) || (ret_len >= len))
    {
        printf("move back failed!\n");
        return;
    }
    else
    {
        left_len = ret_len;
    }

    // rtr bit
    if(0 == strncmp(cursor, "d", CAN_RTR_LEN))
    {
        can_info.rtr = CAN_ANALYZER_DATA_BIT;
        cursor = cursor + CAN_RTR_LEN;
    }
    else if (0 == strncmp(cursor, "r", CAN_RTR_LEN))
    {
        can_info.rtr = CAN_ANALYZER_TX;
        cursor = cursor + CAN_RTR_LEN;
    }
    else
    {
        printf("can direction field error!\n");
        return;
    }
    ret_len = isCorrectFormatAndMoveBackward((uint8_t**)&cursor, (uint8_t**)&last_cursor, left_len);
    if((ret_len <= 0) || (ret_len >= len))
    {
        printf("move back failed!\n");
        return;
    }
    else
    {
        left_len = ret_len;
    }

    // DLC
    can_info.dlc_len.dlc = strtoul(last_cursor, &cursor, CAN_ANALYZER_DECIMAL);
    ret_len = isCorrectFormatAndMoveBackward((uint8_t**)&cursor, (uint8_t**)&last_cursor, left_len);
    if((ret_len <= 0) || (ret_len >= left_len))
    {
        printf("move back failed!\n");
        return;
    }
    else
    {
        left_len = ret_len;
        can_info.dlc_len.length = canfd_dlc_to_len[can_info.dlc_len.dlc];
    }

    // Payload
    unsigned long tmp = 0;
    memset(can_info.payload, 0x00, sizeof(can_info.payload));
    for (uint8_t i = 0; i < can_info.dlc_len.length; i++)
    {
        tmp = strtoul(last_cursor, &cursor, CAN_ANALYZER_HEXADECIMAL);
        if(last_cursor == cursor)
        {
            printf("no data failed.\n");
            return;
        }

        if(tmp > 0xff)
        {
            printf("single over size failed.\n");
            return;
        }

        can_info.payload[i] = (uint8_t)tmp;
        last_cursor = cursor;
    }
    
}