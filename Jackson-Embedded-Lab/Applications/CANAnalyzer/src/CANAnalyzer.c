#include "CANAnalyzer.h"
#include "CommonType.h"
#include "CANAnalyzer_StateMachine.h"
#include "string.h"

// static uint8_t CaseFoldTable[256];

void vdCANAnalyzerInit(ResultCallBackFunc callback)
{
    setCANInfoCallBack(callback);

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
    while(fgets(readbuffer, FILE_READ_BUFFER_LENGTH, fp))
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

    BufferMouse mouse = {0};

    if((NULL == frame) ||  (0 == len))
    {
        printf("Please input valid frame to analyze!\n");
        return;
    }

    mouse.cursor = frame;
    mouse.last_cursor  = frame;
    mouse.left_len = len;
    mouse.ret_len = 0;
    
    wvdTransmitStateTo(CAN_ANALYZER_STATE_IDLE, &can_info, &mouse);

}