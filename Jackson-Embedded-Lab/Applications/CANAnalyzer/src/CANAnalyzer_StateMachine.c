#include "CANAnalyzer_StateMachine.h"
#include "CommonType.h"
#include "stdlib.h"
#include "string.h"

// interface
uint8_t setCANInfoCallBack(ResultCallBackFunc callback);

// internal function
static CANAnalyzerState TransmitStateTo(CANAnalyzerState nextstate, CANAnalyzerInfo *can_info, BufferMouse *mouse);
static ResultCallBackFunc callbackfunc = NULL;

// state machine main
static uint8_t StateMachineMain(CANAnalyzerState state, CANAnalyzerInfo *can_info, BufferMouse *mouse);

// state handler
static CANAnalyzerState IdleState(CANAnalyzerInfo *can_info, BufferMouse *mouse);
static CANAnalyzerState TimestampState(CANAnalyzerInfo *can_info, BufferMouse *mouse);
static CANAnalyzerState ChannelState(CANAnalyzerInfo *can_info, BufferMouse *mouse);
static CANAnalyzerState CANIDState(CANAnalyzerInfo *can_info, BufferMouse *mouse);
static CANAnalyzerState TransDirectionState(CANAnalyzerInfo *can_info, BufferMouse *mouse);
static CANAnalyzerState RTRCheckState(CANAnalyzerInfo *can_info, BufferMouse *mouse);
static CANAnalyzerState DLCState(CANAnalyzerInfo *can_info, BufferMouse *mouse);
static CANAnalyzerState PayloadState(CANAnalyzerInfo *can_info, BufferMouse *mouse);
static CANAnalyzerState BRSState(CANAnalyzerInfo *can_info, BufferMouse *mouse);
static CANAnalyzerState ESIState(CANAnalyzerInfo *can_info, BufferMouse *mouse);
// static CANAnalyzerState ErrorState(CANAnalyzerInfo *can_info, BufferMouse *mouse);
// static CANAnalyzerState FinishState(CANAnalyzerInfo *can_info, BufferMouse *mouse);

static const StateHandlerFunc statemap[] = {
    [CAN_ANALYZER_STATE_ERROR]                = NULL                ,
    [CAN_ANALYZER_STATE_IDLE]                 = IdleState           ,
    [CAN_ANALYZER_STATE_TIMESTAMP]            = TimestampState      ,
    [CAN_ANALYZER_STATE_CHANNEL]              = ChannelState        ,
    [CAN_ANALYZER_STATE_CANID]                = CANIDState          ,
    [CAN_ANALYZER_STATE_TRANSDIRECTION]       = TransDirectionState ,
    [CAN_ANALYZER_STATE_RTRCHECK]             = RTRCheckState       ,
    [CAN_ANALYZER_STATE_DLC]                  = DLCState            ,
    [CAN_ANALYZER_STATE_PAYLOAD]              = PayloadState        ,
    [CAN_ANALYZER_STATE_BRS]                  = BRSState            , // CANFD
    [CAN_ANALYZER_STATE_ESI]                  = ESIState            , // CANFD
    [CAN_ANALYZER_STATE_FINISH]               = NULL                ,
};

static const uint8_t canfd_dlc_to_len[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8,      /* DLC 0~8，for classical can */
    12, 16, 20, 24, 32, 48, 64      /* DLC 9~15，CAN FD专属映射 */
};

uint8_t setCANInfoCallBack(ResultCallBackFunc callback)
{
    callbackfunc = callback;
}

uint8_t wvdTransmitStateTo(CANAnalyzerState nextstate, CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    if((NULL != can_info) && (NULL != mouse))
    {
        return StateMachineMain(nextstate, can_info, mouse);
    }
    else
    {
        printf("invalid input to state machine!\n");
        return RET_NG;
    }
}

static uint8_t StateMachineMain(CANAnalyzerState state, CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    CANAnalyzerState nextstate = state;
    while ((CAN_ANALYZER_STATE_FINISH != nextstate) && (CAN_ANALYZER_STATE_ERROR != nextstate))
    {
        nextstate = TransmitStateTo(nextstate, can_info, mouse);
    }

    if(CAN_ANALYZER_STATE_FINISH == nextstate)
    {
        if(NULL != callbackfunc)
        {
            callbackfunc(*can_info);
        }
        return RET_OK;
    }
    else
    {
        return RET_NG;
    } 
}

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
    if(cur_cursor > last_cursor)
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

        // printf("no timestamp, ignore this item!\n");
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
        // after reading, the first character must be space
        printf("format error, this buffer is not CAN frame!\n");
        printf("total_len: %d, cursor: %d\n", total_len, **cursor);
        return total_len;
    }
    return left_len;
}

static CANAnalyzerState TransmitStateTo(CANAnalyzerState nextstate, CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    StateHandlerFunc handler = statemap[nextstate];
    if (handler == NULL) {
        handler = statemap[CAN_ANALYZER_STATE_ERROR];
    }
    
    return handler(can_info, mouse);
}

// state handler
static CANAnalyzerState ErrorState(CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    // do something
    memset(can_info, 0x00, sizeof(CANAnalyzerInfo));
    memset(mouse, 0x00, sizeof(BufferMouse));

    return RET_OK;
}

static CANAnalyzerState IdleState(CANAnalyzerInfo *can_info, BufferMouse *mouse)
{

    // ignore started space
    mouse->cursor = spaceignore(mouse->cursor);

    if(('\n' == *mouse->cursor) || ('\r' == *mouse->cursor) || ('\0' == *mouse->cursor))
    {
        printf("Empty line!\n");
        return CAN_ANALYZER_STATE_ERROR;
    }

    if(((mouse->left_len >= 2) && ('/' == *mouse->cursor) && ('/' == *(mouse->cursor+1))) || ('#' == *mouse->cursor))
    {
        // printf("ignore comment.\n");
        return CAN_ANALYZER_STATE_ERROR;
    }
    // default standard can
    can_info->type = CAN_ANALYZER_TYPE_CLASSICAL_STANDARD;
    return CAN_ANALYZER_STATE_TIMESTAMP;
}

static CANAnalyzerState TimestampState(CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    // timestamp field
    can_info->timestamp = strtod(mouse->last_cursor, (char**)&mouse->cursor);
    mouse->ret_len = isCorrectFormatAndMoveBackward(&mouse->cursor, &mouse->last_cursor, mouse->left_len);
    if((mouse->ret_len <= 0) || (mouse->ret_len >= mouse->left_len))
    {
        // printf("move back failed!\n");
        return CAN_ANALYZER_STATE_ERROR;
    }
    else
    {
        mouse->left_len = mouse->ret_len;
    }

    return CAN_ANALYZER_STATE_CHANNEL;
}

static CANAnalyzerState ChannelState(CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    // channel field
    if(0 == strncmp(mouse->cursor, "CANFD", strlen("CANFD")))
    {
        can_info->type = CAN_ANALYZER_TYPE_CAN_FD_STANDARD;
        mouse->cursor = mouse->cursor + strlen("CANFD");
        mouse->last_cursor = mouse->cursor;
        return CAN_ANALYZER_STATE_CHANNEL;
    }
    else
    {
        can_info->channel = strtoul(mouse->last_cursor, (char**)&mouse->cursor, CAN_ANALYZER_DECIMAL);
    }

    mouse->ret_len = isCorrectFormatAndMoveBackward(&mouse->cursor, &mouse->last_cursor, mouse->left_len);
    if((mouse->ret_len <= 0) || (mouse->ret_len >= mouse->left_len))
    {
        printf("move back failed!\n");
        return CAN_ANALYZER_STATE_ERROR;
    }
    else
    {
        mouse->left_len = mouse->ret_len;
    }

    // choose next state
    if((can_info->type == CAN_ANALYZER_TYPE_CLASSICAL_STANDARD) || (can_info->type == CAN_ANALYZER_TYPE_CLASSICAL_EXTENDED))
    {
        return CAN_ANALYZER_STATE_CANID;
    }
    else if((can_info->type == CAN_ANALYZER_TYPE_CAN_FD_STANDARD) || (can_info->type == CAN_ANALYZER_TYPE_CAN_FD_EXTENDED))
    {
        return CAN_ANALYZER_STATE_TRANSDIRECTION;
    }
    else
    {
        return CAN_ANALYZER_STATE_ERROR;
    }
    
}

static CANAnalyzerState CANIDState(CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    // CAN ID
    can_info->canid = strtoul(mouse->last_cursor, (char**)&mouse->cursor, CAN_ANALYZER_HEXADECIMAL);
    if(('x' == *mouse->cursor) || ('X' == *mouse->cursor))
    {
        if(can_info->type == CAN_ANALYZER_TYPE_CLASSICAL_STANDARD)
        {
            can_info->type = CAN_ANALYZER_TYPE_CLASSICAL_EXTENDED;
            mouse->cursor++;
        }
        else if(can_info->type == CAN_ANALYZER_TYPE_CAN_FD_STANDARD)
        {
            can_info->type = CAN_ANALYZER_TYPE_CAN_FD_EXTENDED;
            mouse->cursor++;
        }
        else
        {
            printf("Format error!Unknown type with end 'x'\n");
            return CAN_ANALYZER_STATE_ERROR;
        }
    }
    mouse->ret_len = isCorrectFormatAndMoveBackward(&mouse->cursor, &mouse->last_cursor, mouse->left_len);
    if((mouse->ret_len <= 0) || (mouse->ret_len >= mouse->left_len))
    {
        printf("move back failed!\n");
        return CAN_ANALYZER_STATE_ERROR;
    }
    else
    {
        mouse->left_len = mouse->ret_len;
    }

    if ((can_info->type == CAN_ANALYZER_TYPE_CAN_FD_EXTENDED) || (can_info->type == CAN_ANALYZER_TYPE_CAN_FD_STANDARD))
    {
        return CAN_ANALYZER_STATE_BRS;
    }
    else if((can_info->type == CAN_ANALYZER_TYPE_CLASSICAL_EXTENDED) || (can_info->type == CAN_ANALYZER_TYPE_CLASSICAL_STANDARD))
    {
        return CAN_ANALYZER_STATE_TRANSDIRECTION;
    }
    else
    {
        return CAN_ANALYZER_STATE_ERROR;
    }
}

static CANAnalyzerState TransDirectionState(CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    // transmission direction
    if(0 == strncmp((char*)mouse->cursor, "Rx", CAN_TRANSMISSION_DIRECTION_LEN))
    {
        can_info->direction = CAN_ANALYZER_RX;
        mouse->cursor = mouse->cursor + CAN_TRANSMISSION_DIRECTION_LEN;
    }
    else if (0 == strncmp((char*)mouse->cursor, "Tx", CAN_TRANSMISSION_DIRECTION_LEN))
    {
        can_info->direction = CAN_ANALYZER_TX;
        mouse->cursor = mouse->cursor + CAN_TRANSMISSION_DIRECTION_LEN;
    }
    else
    {
        printf("can direction field error!\n");
        return CAN_ANALYZER_STATE_ERROR;
    }
    mouse->ret_len = isCorrectFormatAndMoveBackward(&mouse->cursor, &mouse->last_cursor, mouse->left_len);
    if((mouse->ret_len <= 0) || (mouse->ret_len >= mouse->left_len))
    {
        printf("move back failed!\n");
        return CAN_ANALYZER_STATE_ERROR;
    }
    else
    {
        mouse->left_len = mouse->ret_len;
    }

    if (can_info->type == CAN_ANALYZER_TYPE_CAN_FD_STANDARD)
    {
        return CAN_ANALYZER_STATE_CANID;
    }
    else
    {
        return CAN_ANALYZER_STATE_RTRCHECK;
    }
}

static CANAnalyzerState RTRCheckState(CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    // rtr bit
    if(0 == strncmp((char*)mouse->cursor, "d", CAN_RTR_LEN))
    {
        can_info->rtr = CAN_ANALYZER_DATA_BIT;
        mouse->cursor = mouse->cursor + CAN_RTR_LEN;
    }
    else if (0 == strncmp((char*)mouse->cursor, "r", CAN_RTR_LEN))
    {
        can_info->rtr = CAN_ANALYZER_REMOTE_BIT;
        mouse->cursor = mouse->cursor + CAN_RTR_LEN;
    }
    else
    {
        printf("can direction field error!\n");
        return CAN_ANALYZER_STATE_ERROR;
    }
    mouse->ret_len = isCorrectFormatAndMoveBackward(&mouse->cursor, &mouse->last_cursor, mouse->left_len);
    if((mouse->ret_len <= 0) || (mouse->ret_len >= mouse->left_len))
    {
        printf("move back failed!\n");
        return CAN_ANALYZER_STATE_ERROR;
    }
    else
    {
        mouse->left_len = mouse->ret_len;
    }

    return CAN_ANALYZER_STATE_DLC;
}

static CANAnalyzerState DLCState(CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    // DLC
    can_info->dlc_len.dlc = strtoul(mouse->last_cursor, (char**)&mouse->cursor, CAN_ANALYZER_HEXADECIMAL);
    mouse->ret_len = isCorrectFormatAndMoveBackward(&mouse->cursor, &mouse->last_cursor, mouse->left_len);
    if((mouse->ret_len <= 0) || (mouse->ret_len >= mouse->left_len))
    {
        printf("move back failed!\n");
        return CAN_ANALYZER_STATE_ERROR;
    }
    else
    {
        mouse->left_len = mouse->ret_len;
        if(can_info->dlc_len.dlc < 16)
        {
            can_info->dlc_len.length = canfd_dlc_to_len[can_info->dlc_len.dlc];
        }
        else
        {
            return CAN_ANALYZER_STATE_ERROR;
        }
    }

    if((CAN_ANALYZER_TYPE_CAN_FD_EXTENDED == can_info->type) || (CAN_ANALYZER_TYPE_CAN_FD_STANDARD == can_info->type))
    {
        // in CANFD, the bit is actual len in decimal, which should equal to decoded DLC.
        int data_len = 0;
        data_len = strtoul(mouse->last_cursor, (char**)&mouse->cursor, CAN_ANALYZER_DECIMAL);
        
        mouse->ret_len = isCorrectFormatAndMoveBackward(&mouse->cursor, &mouse->last_cursor, mouse->left_len);
        if((mouse->ret_len <= 0) || (mouse->ret_len >= mouse->left_len))
        {
            printf("move back failed!\n");
            return CAN_ANALYZER_STATE_ERROR;
        }
        else
        {
            if(data_len != can_info->dlc_len.length)
            {
                printf("CANID: %lx, frame error!DLC in CANFD can't match buffer length!\n",can_info->canid);
                return CAN_ANALYZER_STATE_ERROR;
            }
            else
            {
                // frame correct, go to payload.
            }
        }
    }
    return CAN_ANALYZER_STATE_PAYLOAD;
}

static CANAnalyzerState PayloadState(CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    // Payload
    unsigned long tmp = 0;
    memset(can_info->payload, 0x00, sizeof(can_info->payload));
    for (uint8_t i = 0; i < can_info->dlc_len.length; i++)
    {
        tmp = strtoul(mouse->last_cursor, (char**)&mouse->cursor, CAN_ANALYZER_HEXADECIMAL);
        if(mouse->last_cursor == mouse->cursor)
        {
            printf("no data failed.\n");
            return CAN_ANALYZER_STATE_ERROR;
        }

        if(tmp > 0xff)
        {
            printf("single over size failed.\n");
            return CAN_ANALYZER_STATE_ERROR;
        }

        can_info->payload[i] = (uint8_t)tmp;
        mouse->last_cursor = mouse->cursor;
    }

    return CAN_ANALYZER_STATE_FINISH;
}

static CANAnalyzerState BRSState(CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    // printf("BRSState\n");
    can_info->brs = strtoul(mouse->last_cursor, (char**)&mouse->cursor, CAN_ANALYZER_DECIMAL);

    mouse->ret_len = isCorrectFormatAndMoveBackward(&mouse->cursor, &mouse->last_cursor, mouse->left_len);
    if((mouse->ret_len <= 0) || (mouse->ret_len >= mouse->left_len))
    {
        printf("move back failed!\n");
        return CAN_ANALYZER_STATE_ERROR;
    }
    else
    {
        mouse->left_len = mouse->ret_len;
    }

    return CAN_ANALYZER_STATE_ESI;

}

static CANAnalyzerState ESIState(CANAnalyzerInfo *can_info, BufferMouse *mouse)
{
    // printf("ESIState\n");
    can_info->esi = strtoul(mouse->last_cursor, (char**)&mouse->cursor, CAN_ANALYZER_DECIMAL);

    mouse->ret_len = isCorrectFormatAndMoveBackward(&mouse->cursor, &mouse->last_cursor, mouse->left_len);
    if((mouse->ret_len <= 0) || (mouse->ret_len >= mouse->left_len))
    {
        printf("move back failed!\n");
        return CAN_ANALYZER_STATE_ERROR;
    }
    else
    {
        mouse->left_len = mouse->ret_len;
    }

    return CAN_ANALYZER_STATE_DLC;

}

