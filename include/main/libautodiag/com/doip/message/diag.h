#ifndef __AD_COM_DOIP_MESSAGE_DIAG_H
#define __AD_COM_DOIP_MESSAGE_DIAG_H

#include "libautodiag/lib.h"
#include "libautodiag/com/doip/doip.h"

AD_OBJECT_H(DoIPMessagePayloadDiag,
    DoIPMessageDef;
    Buffer * src_addr;
    Buffer * dst_addr;
    Buffer * data;
)
#define DOIP_MESSAGE_DIAG_ADDR_SZ 2
/**
 * Creates a diagnostic message.
 */
ad_object_DoIPMessage * doip_message_diag(Buffer * to, Buffer * from, Buffer * payload_data);

AD_OBJECT_H(DoIPMessagePayloadDiagFeedback,
    DoIPMessageDef;
    Buffer * src_addr;
    Buffer * dst_addr;
    byte code;
    Buffer * data;
    uint16_t type;
)
typedef enum {
    DOIP_DIAGNOSTIC_MESSAGE_ACK_CODE_POSITIVE = 0x00
    // 0x01-0xFF reserved
} DOIP_DIAGNOSTIC_MESSAGE_ACK_CODE;

ad_object_DoIPMessage * doip_message_diag_feedback_ack(Buffer * to, Buffer * from, Buffer * payload_data, byte code);

typedef enum {
    // 0x00 0x01 reserved
    DOIP_DIAGNOSTIC_MESSAGE_NACK_CODE_INV_SOURCE_ADDR = 0x02,
    DOIP_DIAGNOSTIC_MESSAGE_NACK_CODE_UNKNOWN_TARGET_ADDR = 0x03,
    DOIP_DIAGNOSTIC_MESSAGE_NACK_CODE_MSG_TOO_LARGE = 0x04,
    // Out Of Memory
    DOIP_DIAGNOSTIC_MESSAGE_NACK_CODE_OOM = 0x05,
    DOIP_DIAGNOSTIC_MESSAGE_NACK_CODE_TARGET_UNREACH = 0x06,
    DOIP_DIAGNOSTIC_MESSAGE_NACK_CODE_UNKNOWN_NET = 0x07,
    DOIP_DIAGNOSTIC_MESSAGE_NACK_CODE_TRANSPORT_PROTO_ERROR = 0x08
    // 0x09-0xFF reserved
} DOIP_DIAGNOSTIC_MESSAGE_NACK_CODE;
ad_object_DoIPMessage * doip_message_diag_feedback_nack(Buffer * to, Buffer * from, Buffer * payload_data, byte code);

#endif