#ifndef __COM_DOIP_H
#define __COM_DOIP_H

/**
 * ISO 13400
 */

#include "libautodiag/lib.h"

typedef enum {
    DOIP_GENERIC_HEADER_NACK            = 0x0000, /* Generic DoIP header negative acknowledgment */
    DOIP_VEHICLE_IDENT_REQUEST          = 0x0001, /* Vehicle identification request */
    DOIP_VEHICLE_IDENT_REQUEST_EID      = 0x0002, /* Vehicle identification request with EID */
    DOIP_VEHICLE_IDENT_REQUEST_VIN      = 0x0003, /* Vehicle identification request with VIN */
    DOIP_VEHICLE_ANNOUNCEMENT_RESPONSE  = 0x0004, /* Vehicle announcement / identification response */
    DOIP_ROUTING_ACTIVATION_REQUEST     = 0x0005, /* Routing activation request */
    DOIP_ROUTING_ACTIVATION_RESPONSE    = 0x0006, /* Routing activation response */
    DOIP_ALIVE_CHECK_REQUEST            = 0x0007, /* Alive check request */
    DOIP_ALIVE_CHECK_RESPONSE           = 0x0008, /* Alive check response */
    DOIP_ENTITY_STATUS_REQUEST          = 0x4001, /* DoIP entity status request */
    DOIP_ENTITY_STATUS_RESPONSE         = 0x4002, /* DoIP entity status response */
    DOIP_DIAG_POWER_MODE_REQUEST        = 0x4003, /* Diagnostic power mode info request */
    DOIP_DIAG_POWER_MODE_RESPONSE       = 0x4004, /* Diagnostic power mode info response */
    DOIP_DIAGNOSTIC_MESSAGE             = 0x8001, /* Diagnostic message (UDS payload) */
    DOIP_DIAGNOSTIC_MESSAGE_ACK         = 0x8002, /* Diagnostic message positive ACK */
    DOIP_DIAGNOSTIC_MESSAGE_NACK        = 0x8003  /* Diagnostic message negative ACK */
} DoIpPayloadType;
OBJECT_H(DoIPMessage,
    byte protocol_version;
    byte inv_protocol_version;
    DoIpPayloadType payload_type;
    Buffer * payload_raw;
)
OBJECT_H(DoIPDiagMessage,
    object_DoIPMessage;
    struct {
        Buffer * src_addr;
        Buffer * dst_addr;
        Buffer * data;
    } payload;
)

#define DOIP_PROTOCOL_VERSION_CURRENT 0x02

object_DoIPDiagMessage * object_DoIPDiagMessage_new();
object_DoIPDiagMessage *doip_diag_message_parse(const Buffer *in);
Buffer *doip_diag_message_serialize(const object_DoIPDiagMessage *msg);
object_DoIPMessage * doip_message_parse(const Buffer * in);
void doip_diag_message_dump(object_DoIPDiagMessage * msg);

#include "libautodiag/com/doip/device.h"

#endif