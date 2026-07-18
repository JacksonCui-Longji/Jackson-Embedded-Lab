#include "CANAnalyzer.h"
#include "CommonType.h"
#include "CANAnalyzer_StateMachine.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <unistd.h>
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

uint8_t CANSocketToCANInfo(struct can_frame *frame, CANAnalyzerInfo *info)
{
    if((NULL == frame) || (NULL == info))
    {
        return RET_NG;
    }

    memset(info, 0, sizeof(CANAnalyzerInfo));

    uint8_t is_extended = (frame->can_id & CAN_EFF_FLAG) ? 1 : 0;
    info->canid = frame->can_id & (is_extended ? CAN_EFF_MASK : CAN_SFF_MASK);

    info->rtr = (frame->can_id & CAN_RTR_FLAG) ? CAN_ANALYZER_REMOTE_BIT : CAN_ANALYZER_DATA_BIT;

    info->type = is_extended ? CAN_ANALYZER_TYPE_CLASSICAL_EXTENDED : CAN_ANALYZER_TYPE_CLASSICAL_STANDARD;

    info->dlc_len.dlc    = frame->can_dlc;
    info->dlc_len.length = frame->can_dlc;   // Classical CAN: DLC == length，二者一致(0~8)

    if(frame->can_dlc > CAN_ANALYZER_PAYLOAD_MAX_LEN)
    {
        return RET_NG;
    }
    memcpy(info->payload, frame->data, frame->can_dlc);

    info->direction = CAN_ANALYZER_RX;

    return RET_OK;
}

static uint8_t CANFDSocketToCANInfo(struct canfd_frame *frame, CANAnalyzerInfo *info)
{
    if ((NULL == frame) || (NULL == info)) 
    {
        return RET_NG;
    }

    memset(info, 0x00, sizeof(CANAnalyzerInfo));

    uint8_t is_extended = (frame->can_id & CAN_EFF_FLAG) ? 1 : 0;
    info->canid = frame->can_id & (is_extended ? CAN_EFF_MASK : CAN_SFF_MASK);

    info->rtr = CAN_ANALYZER_DATA_BIT; // no concept of rtr in CANFD, so permanent data bit.

    info->type = is_extended ? CAN_ANALYZER_TYPE_CAN_FD_EXTENDED : CAN_ANALYZER_TYPE_CAN_FD_STANDARD;

    info->dlc_len.length = frame->len;

    info->brs = (frame->flags & CANFD_BRS) ? 1 : 0;
    info->esi = (frame->flags & CANFD_ESI) ? 1 : 0;

    memcpy(info->payload, frame->data, frame->len);
    info->direction = CAN_ANALYZER_RX;

    return 1;
}

void vdCANAnalyzeSocketCAN(const char *ifname)
{
    if(NULL == ifname)
    {
        printf("Please input valid interface name!\n");
        return;
    }

    int sock_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if(sock_fd < 0)
    {
        perror("socket");
        return;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if(ioctl(sock_fd, SIOCGIFINDEX, &ifr) < 0)
    {
        perror("ioctl SIOCGIFINDEX");
        close(sock_fd);
        return;
    }

    struct sockaddr_can addr = {0};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if(bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(sock_fd);
        return;
    }
    
    int enable_fd = 1;
    if(setsockopt(sock_fd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_fd, sizeof(enable_fd)))
    {
        perror("setsockopt CAN_RAW_FD_FRAMES");
        close(sock_fd);
        return;
    }
    
    printf("Listening on %s ... (Ctrl+C to stop)\n", ifname);

    struct canfd_frame frame;
    CANAnalyzerInfo can_info;
    
    while(1)
    {
        ssize_t nbytes = read(sock_fd, &frame, sizeof(frame));
        if(nbytes < 0)
        {
            perror("read");
            break;
        }
        if (nbytes == CAN_MTU)
        {
            struct can_frame *classic = (struct can_frame *)&frame;
            ResultCallBackFunc callback = getCANInfoCallBack();
            if((RET_OK == CANSocketToCANInfo(classic, &can_info)) && (NULL != callback))
            {
                callback(can_info);
            }
        }
        else if (nbytes == CANFD_MTU)
        {
            ResultCallBackFunc callback = getCANInfoCallBack();
            if((RET_OK == CANFDSocketToCANInfo(&frame, &can_info)) && (NULL != callback))
            {
                callback(can_info);
            }

        }
        else
        {
            printf("Unexpected frame size: %zd\n", nbytes);
            continue;
        }
    }
    close(sock_fd);
}