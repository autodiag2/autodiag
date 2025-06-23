/**
 * ISO 15765 also called ISO-TP - used to transfer more than eight bytes on it
 * It operate over a can bus
 */
#ifndef __iso15765_2_H
#define __iso15765_2_H

#include "can.h"
#include "libautodiag/lib.h"
#include "libautodiag/buffer.h"
#include "libautodiag/model/vehicle.h"

typedef enum {
    Iso15765ControlFrameFlowStatus_ClearToSend,
    Iso15765ControlFrameFlowStatus_Wait,
    Iso15765ControlFrameFlowStatus_OverLoad
}Iso15765ControlFrameFlowStatus;
#define Iso15765ControlFrameFlowStatus_MAX Iso15765ControlFrameFlowStatus_OverLoad
#define ISO_15765_SINGLE_FRAME_DATA_BYTES 7

typedef enum {
    Iso15765SingleFrame,
    Iso15765FirstFrame,
    Iso15765ConsecutiveFrame,
    Iso15765FlowControlFrame
}Iso15765FrameType;

/**
 * Store the conversation with a given ECU.
 */
typedef struct {
    /**
     * Remaining data bytes to receive in this conversation
     */
    int remaining_data_bytes_to_receive;
    /**
     * Data buffer of the conversation
     */
    Buffer * data;
    /**
     * sequence serial number of the currently proceceed message (or 0 if only one kind of trame)
     */
    int current_sn;
    /**
     * effective length of the data of the currently proceceed message
     */
    int current_data_length;
    /**
     * ECU that has initiated this conversation, this is only a pointer, the associtated ecu
     * should not be free after use of the conversation
     */
    ECU * ecu;
} Iso15765Conversation;

Iso15765Conversation* iso15765_init_conversation(int expected_bytes);
void iso15765_conversation_free(Iso15765Conversation *conv);

LIST_H(Iso15765Conversation)
Iso15765Conversation* list_Iso15765Conversation_find(list_Iso15765Conversation* list, ECU* ecu);

#endif
