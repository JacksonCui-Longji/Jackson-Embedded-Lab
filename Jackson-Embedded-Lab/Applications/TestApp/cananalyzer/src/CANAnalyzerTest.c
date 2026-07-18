#include "CANAnalyzer.h"
#include "string.h"
#include "getopt.h"
#include "stdio.h"

void vdCANAnalyzerCallback(CANAnalyzerInfo can_info)
{
    uint8_t *type = NULL;
    switch (can_info.type)
    {
        case CAN_ANALYZER_TYPE_CLASSICAL_STANDARD:
        {
            type = "Standard CAN";
            break;
        }
        case CAN_ANALYZER_TYPE_CLASSICAL_EXTENDED:
        {
            type = "Extended CAN";
            break;
        }
        case CAN_ANALYZER_TYPE_CAN_FD_STANDARD:
        {
            type = "Standard CAN FD";
            break;
        }
        case CAN_ANALYZER_TYPE_CAN_FD_EXTENDED:
        {
            type = "Extended CAN FD";
            break;
        }
        default:
        {
            break;
        }
    }

    uint8_t *direction = NULL;
    switch (can_info.direction)
    {
        case CAN_ANALYZER_RX:
        {
            direction = "Rx";
            break;
        }
        case CAN_ANALYZER_TX:
        {
            direction = "Tx";
            break;
        }
        default:
        {
            break;
        }
    }
    uint8_t *rtr;
    switch (can_info.rtr)
    {
        case CAN_ANALYZER_DATA_BIT:
        {
            rtr = "Data";
            break;
        }
        case CAN_ANALYZER_REMOTE_BIT:
        {
            rtr = "Remote";
            break;
        }
        default:
        {
            break;
        }
    }

    printf("--------------%s--------------\n", __func__);
    printf("CAN Type: %s\n", type);
    printf("TimeStamp: %f, channel: %ld, canid: %lX\n", can_info.timestamp, can_info.channel, can_info.canid);
    printf("CANDirection: %s, RTR: %s, DLC(length): %d\n", direction, rtr, can_info.dlc_len.length);
    printf("payload: \n");
    for(int i = 0; i < can_info.dlc_len.length; i++)
    {
        printf("%02X ", can_info.payload[i]);
    }
    printf("\n");
}

void vdCANAnalyzeSocket(const char *ifname)
{
    if(NULL != ifname)
    {
        vdCANAnalyzeSocketCAN(ifname);
    }
    return;
}

void vdCANAnalyzeCommandLine(void)
{
    char inBuffer[1024] = {0};
    printf("Please input ASC format CAN Frame!\n");

    while(NULL != fgets(inBuffer, sizeof(inBuffer), stdin))
    {
        if((0 == strncmp(inBuffer, "quit", strlen("quit"))) || (0 == strncmp(inBuffer, "q", strlen("q"))))
        {
            break;
        }
        else
        {
            vdCanFrameAnalyze(inBuffer, strlen(inBuffer));
        }
        printf("Please input ASC format CAN Frame!\n");
    }

    return;
}

void vdCANAnalyzeASCFile(char *path)
{
    uint8_t buffer[1024] = {0};

    // printf("vdCANAnalyzeASCFile\n");
    if(NULL == path)
    {
        printf("Please input valid path! \n");
        return;
    }

    size_t len = strlen(path);
    if (len > 0 && (('\r' == path[len - 1]) || ('\n' == path[len - 1])))
    {
        path[len - 1] = '\0';
        len--;
    }

    FILE *fp = NULL;

    fp = fopen(path, "r");
    if (NULL == fp)
    {
        printf("Failed to open file: %s\n", path);
        return;
    }
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        vdCanFrameAnalyze(buffer, strlen(buffer));
        memset(buffer, 0x00, sizeof(buffer));
    }

}

static struct option long_option[] = {
    {"path", required_argument, NULL, 'p'},
    {"string", no_argument, NULL, 's'},
    {"can", required_argument, NULL, 'c'},
};

int main(int argc, char *argv[])
{
    int opt;

    // printf("CAN AnalyzerTest\n");

    vdCANAnalyzerInit(vdCANAnalyzerCallback);

    while((opt = getopt_long(argc, argv, "c:p:sh", long_option, NULL)) != -1)
    {
        switch (opt)
        {
            case 'p':
            {
                char *path = optarg;
                vdCANAnalyzeASCFile(path);
                break;
            }
            case 's':
            {
                vdCANAnalyzeCommandLine();
                break;
            }
            case 'c':
            {
                char *device = optarg;
                vdCANAnalyzeSocket(device);
                break;
            }
            case 'h':
            {
                printf("Usage: CanAnalyzerTest -p/s/c/h\n");
                printf("        -h\n");
                printf("        -p <filepath>\n");
                printf("        -s <asc format string>\n");
                printf("        -c <listen device name>\n");
                break;
            }
            case '?':
            {
                printf("Usage: -h for help\n");
                break;
            }
            default:
            {
                printf("Usage: -h for help\n");
                break;
            }
        }
    }

    return 0;
}