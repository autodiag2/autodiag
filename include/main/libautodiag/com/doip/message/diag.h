#ifndef __COM_DOIP_MESSAGE_DIAG_H
#define __COM_DOIP_MESSAGE_DIAG_H

#include "libautodiag/lib.h"
#include "libautodiag/com/doip/doip.h"

OBJECT_H(DoIPMessagePayloadDiag,
    DoIPMessageDef;
    Buffer * src_addr;
    Buffer * dst_addr;
    Buffer * data;
)
#define DOIP_MESSAGE_DIAG_ADDR_SZ 2
/**
 * Creates a diagnostic message.
 */
object_DoIPMessage * doip_diag_message(Buffer * to, Buffer * from, Buffer * payload_data);

typdef enum {
    DOIP_MESSAGE_DIAG_FEEDBACK_ACK = 0x00,
    DOIP_MESSAGE_DIAG_FEEDBACK_NACK = 0x01
} DOIP_MESSAGE_DIAG_FEEDBACK_TYPE;

OBJECT_H(DoIPMessagePayloadDiagFeedback,
    DoIPMessageDef;
    Buffer * src_addr;
    Buffer * dst_addr;
    byte code;
    Buffer * data;
    DOIP_MESSAGE_DIAG_FEEDBACK_TYPE type;
)
typedef enum {
    DOIP_DIAGNOSTIC_MESSAGE_ACK_CODE_POSITIVE = 0x00
    // 0x01-0xFF reserved
} DOIP_DIAGNOSTIC_MESSAGE_ACK_CODE;

#endif