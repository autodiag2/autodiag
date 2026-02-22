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

typdef struct {

} DoIPMessageDef;

OBJECT_H(DoIPMessage,
    byte protocol_version;
    byte inv_protocol_version;
    DoIpPayloadType payload_type;
    Buffer * payload_raw;
    DoIPMessageDef * payload;
)
#define DOIP_PROTOCOL_VERSION_CURRENT 0x02
object_DoIPMessage * doip_message_parse(const Buffer * in);
Buffer * doip_message_serialize(const object_DoIPMessage * in);
void doip_message_dump(final object_DoIPMessage * msg);
void doip_message_init(final object_DoIPMessage * msg, final DoIpPayloadType type);

OBJECT_H(DoIPMessagePayloadDiag,
    DoIPMessageDef;
    Buffer * src_addr;
    Buffer * dst_addr;
    Buffer * data;
)
#define DOIP_MESSAGE_DIAG_ADDR_SZ 2
object_DoIPMessagePayloadDiag * object_DoIPMessagePayloadDiag_new();
object_DoIPMessage * doip_diag_message(Buffer * to, Buffer * from, Buffer * payload_data);

typdef enum {
    DOIP_MESSAGE_RAR_TYPE_DEFAULT = 0x00,
    DOIP_MESSAGE_RAR_TYPE_WWH_OBD = 0x01,
    DOIP_MESSAGE_RAR_TYPE_ISO_SAE_RESERVED = 0x02,
    DOIP_MESSAGE_RAR_TYPE_CENTRAL_SECURITY = 0xE0,
    DOIP_MESSAGE_RAR_TYPE_OEM = 0x00
} DOIP_MESSAGE_RAR_TYPE;

OBJECT_H(DoIPMessagePayloadRoutineActivationRequest,
    DoIPMessageDef;
    byte src_addr[2];
    DOIP_MESSAGE_RAR_TYPE activation_type;
    byte iso_reserved[4];
    byte vendor_reserved[4];
    Buffer * dst_addr;
    Buffer * data;
)

#include "libautodiag/com/doip/device.h"

#endif