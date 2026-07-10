#ifndef _HEX_TOOL_
#define _HEX_TOOL_

#include "stdint.h"
#include "stdio.h"

#define MAX_INPUT_STRING_LENGTH 0xffffu

extern uint8_t u8HexToolRead(uint8_t *inPath, size_t len);
extern uint8_t u8HexToolWrite(uint8_t *inData, size_t len);

#endif // _HEX_TOOL_