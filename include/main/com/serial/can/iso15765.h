/**
 * Also called ISO-TP - used to transfer more than eight bytes on it
 * It operate over a can bus
 */
#ifndef __iso15765_2_H
#define __iso15765_2_H

#include "can.h"

typedef enum {
    Iso15765ControlFrameFlowStatus_ClearToSend,
    Iso15765ControlFrameFlowStatus_Wait,
    Iso15765ControlFrameFlowStatus_OverLoad
}Iso15765ControlFrameFlowStatus;
#define Iso15765ControlFrameFlowStatus_MAX Iso15765ControlFrameFlowStatus_OverLoad

typedef enum {
    Iso15765SingleFrame,
    Iso15765FirstFrame,
    Iso15765ConsecutiveFrame,
    Iso15765FlowControlFrame
}Iso15765FrameType;

#endif
