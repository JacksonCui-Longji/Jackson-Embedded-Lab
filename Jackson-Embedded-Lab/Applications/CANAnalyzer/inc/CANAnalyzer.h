#ifndef _CAN_ANALYZER_H_
#define _CAN_ANALYZER_H_

#include <linux/can.h>
#include <linux/can/raw.h>
#include "stdint.h"
#include "stdio.h"

// input CAN Frame from stdin/file
// ignore empty lines,space
// ignore commonts such as #, // and /* */

extern void vdCANAnalyzeFile(uint8_t *path, size_t path_len);

extern void vdCanFrameAnalyze(uint8_t *frame, size_t len);

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
