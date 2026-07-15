#ifndef _CANANALYZER_STATEMACHINE_
#define _CANANALYZER_STATEMACHINE_

#include "stdint.h"
#include "CANAnalyzer.h"

typedef struct _BufferMouse_
{
    uint8_t *cursor;
    uint8_t *last_cursor;
    int left_len;
    int ret_len;

}BufferMouse;

typedef enum _CANAnalayzerState_
{
    CAN_ANALYZER_STATE_ERROR            = 0x00,
    CAN_ANALYZER_STATE_IDLE             = 0x01,
    CAN_ANALYZER_STATE_TIMESTAMP        = 0x02,
    CAN_ANALYZER_STATE_CHANNEL          = 0x03,
    CAN_ANALYZER_STATE_CANID            = 0x04,
    CAN_ANALYZER_STATE_TRANSDIRECTION   = 0x05,
    CAN_ANALYZER_STATE_RTRCHECK         = 0x06,
    CAN_ANALYZER_STATE_DLC              = 0x07,
    CAN_ANALYZER_STATE_PAYLOAD          = 0x08,
    CAN_ANALYZER_STATE_FINISH           = 0x09,

}CANAnalyzerState;

typedef CANAnalyzerState (*StateHandlerFunc)(CANAnalyzerInfo *can_info, BufferMouse *mouse);




// outside interface
extern uint8_t wvdTransmitStateTo(CANAnalyzerState nextstate, CANAnalyzerInfo *can_info, BufferMouse *mouse);


/*   when the CANAnalyzerState is continuously, there's no need to make a table.
typedef struct _CanAnalyzerStateMachineMap_
{
    CANAnalyzerState state;
    StateHandlerFunc handler;
}CanAnalyzerStateMachineMap;
*/
#endif // _CANANALYZER_STATEMACHINE_