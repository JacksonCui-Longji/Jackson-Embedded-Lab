#ifndef _CAN_ANALYZER_H_
#define _CAN_ANALYZER_H_

#include "stdint.h"
#include "stdio.h"

#define FILE_READ_BUFFER_LENGTH         0x400u
#define CAN_ANALYZER_DECIMAL            (10)
#define CAN_ANALYZER_HEXADECIMAL        (16)
#define CAN_TRANSMISSION_DIRECTION_LEN  (2)
#define CAN_RTR_LEN                     (1)
#define CAN_ANALYZER_PAYLOAD_MAX_LEN    (64)


typedef enum _CanType_{
    CAN_ANALYZER_TYPE_CLASSICAL_STANDARD    = 0x00,
    CAN_ANALYZER_TYPE_CLASSICAL_EXTENDED    = 0x01,
    CAN_ANALYZER_TYPE_CAN_FD_STANDARD       = 0x02,
    CAN_ANALYZER_TYPE_CAN_FD_EXTENDED       = 0x03,
}CanType;

typedef enum _CanDirection_{
    CAN_ANALYZER_RX = 0x00,
    CAN_ANALYZER_TX = 0x01,
}CanDirection;

typedef enum _CanRTRBit_{
    CAN_ANALYZER_DATA_BIT = 0x00,
    CAN_ANALYZER_REMOTE_BIT = 0x01,
}CanRTRBit;

typedef struct _CanDLC_{
    uint8_t dlc;
    uint8_t length;
}CanDLC;

typedef struct _CANAnalyzerInfo_
{
    CanType type;
    double timestamp;
    unsigned long channel;
    unsigned long canid;
    CanDirection direction;
    CanRTRBit rtr;
    CanDLC dlc_len;
    uint8_t brs;
    uint8_t esi;
    uint8_t payload[CAN_ANALYZER_PAYLOAD_MAX_LEN];
}CANAnalyzerInfo;


// input CAN Frame from stdin/file
// ignore empty lines,space
// ignore commonts such as #, // and /* */

extern void vdCANAnalyzerInit();

extern void vdCANAnalyzeFile(uint8_t *path, size_t path_len);

extern void vdCanFrameAnalyze(uint8_t *frame, int len);

extern void vdCANAnalyzeSocketCAN(const char *ifname);


// detect  CAN ID type: Standard ID / Extended ID / 

// Display frame information
/*
    Timestamp:
        eg: Current device timestamp
    CAN ID
        eg: 0x18DAF110
    Frame Type
        eg: Extended
    DLC
        eg: 8
    Payload
        eg: 02 1003 55 55 55 55 55
*/

// validate CAN ID: 0x000~0x7FF, or 0x00000000~0x1FFFFFFF

// validate DLC: 0~8

// validdate Data length, use DLC check the frame is validate or not.

// Display every payload bit, with input 11, output 00010001 for every payload

// display in hex

// display in decimal

// display in ASCII



#endif // _CAN_ANALYZER_H_
