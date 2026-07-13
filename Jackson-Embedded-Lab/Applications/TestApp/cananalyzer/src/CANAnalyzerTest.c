#include "CANAnalyzer.h"
#include "string.h"


void vdCANAnalyzeCommandLine()
{

}

int main()
{
    printf("CAN AnalyzerTest\n");
    uint8_t buffer[255] = {0};

    vdCANAnalyzerInit();

    vdCanFrameAnalyze(buffer, strlen(buffer));
    return 0;
}