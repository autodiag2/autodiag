#ifndef __AD_COM_DOIP_H
#define __AD_COM_DOIP_H

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

AD_OBJECT_H(DoIPMessagePayloadEmpty,
    DoIPMessageDef;
)

AD_OBJECT_H(DoIPMessage,
    byte protocol_version;
    byte inv_protocol_version;
    DoIpPayloadType payload_type;
    Buffer * payload_raw;
    DoIPMessageDef * payload;
)
#define DOIP_PROTOCOL_VERSION_CURRENT 0x02
ad_object_DoIPMessage * doip_message_parse(const Buffer * in);
Buffer * doip_message_serialize(const ad_object_DoIPMessage * in);
void doip_message_dump(final ad_object_DoIPMessage * msg);
void doip_message_init(final ad_object_DoIPMessage * msg, final DoIpPayloadType type);
/**
 * Init with a given type of message
 */
ad_object_DoIPMessage * doip_message_new(DoIpPayloadType type);

typdef enum {
    DOIP_MESSAGE_RAR_TYPE_DEFAULT = 0x00,
    DOIP_MESSAGE_RAR_TYPE_WWH_OBD = 0x01,
    DOIP_MESSAGE_RAR_TYPE_ISO_SAE_RESERVED = 0x02,
    DOIP_MESSAGE_RAR_TYPE_CENTRAL_SECURITY = 0xE0,
    DOIP_MESSAGE_RAR_TYPE_OEM = 0x00
} DOIP_MESSAGE_RAR_TYPE;

AD_OBJECT_H(DoIPMessagePayloadRoutineActivationRequest,
    DoIPMessageDef;
    byte src_addr[2];
    DOIP_MESSAGE_RAR_TYPE activation_type;
    byte iso_reserved[4];
    byte vendor_reserved[4];
)

typedef enum {
    DOIP_MESSAGE_RARES_CODE_UNKNOWN_SOURCE_ADDRESS                   = 0x00,
    DOIP_MESSAGE_RARES_CODE_ALL_SOCKETS_REGISTERED_AND_ACTIVE        = 0x01,
    DOIP_MESSAGE_RARES_CODE_DIFFERENT_SA_ON_ALREADY_ACTIVATED_SOCKET = 0x02,
    DOIP_MESSAGE_RARES_CODE_SA_ALREADY_ACTIVE_ON_DIFFERENT_SOCKET    = 0x03,
    DOIP_MESSAGE_RARES_CODE_MISSING_AUTHENTICATION                   = 0x04,
    DOIP_MESSAGE_RARES_CODE_REJECTED_CONFIRMATION                    = 0x05,
    DOIP_MESSAGE_RARES_CODE_UNSUPPORTED_ROUTING_ACTIVATION_TYPE      = 0x06,

    DOIP_MESSAGE_RARES_CODE_RESERVED_ISO_13400_LOW                   = 0x07,

    DOIP_MESSAGE_RARES_CODE_SUCCESS                                  = 0x10,
    DOIP_MESSAGE_RARES_CODE_CONFIRMATION_REQUIRED                    = 0x11,

    DOIP_MESSAGE_RARES_CODE_RESERVED_ISO_13400_MID                   = 0x12,

    DOIP_MESSAGE_RARES_CODE_OEM_SPECIFIC                             = 0xE0,

    DOIP_MESSAGE_RARES_CODE_RESERVED_ISO_13400_HIGH                  = 0xFF,

    DOIP_MESSAGE_RARES_CODE_UNKNOWN                                  = 0x100
} DOIP_MESSAGE_RARES_CODE;

/**
 * String explanation of code
 */
char *doip_message_rares_code_to_string(unsigned code);
/**
 * Raw approximative of the code
 */
DOIP_MESSAGE_RARES_CODE doip_message_rares_n_to_code(unsigned rares_n);

AD_OBJECT_H(DoIPMessagePayloadRoutineActivationResponse,
    Buffer * tester;
    Buffer * ecu;
    unsigned code;
    Buffer * iso_reserved;
    Buffer * oem_reserved;
)

AD_OBJECT_H(DoIPMessagePayloadAliveCheck,
    DoIPMessageDef;
    Buffer * src_addr;
)

typdef enum {
    DOIP_MESSAGE_ENTITY_NODE_TYPE_GATEWAY = 0x00,
    DOIP_MESSAGE_ENTITY_NODE_TYPE_ECU = 0x01,
    DOIP_MESSAGE_ENTITY_NODE_TYPE_RESERVED = 0x02,
    DOIP_MESSAGE_ENTITY_NODE_TYPE_UNSET = 0x03
} DOIP_MESSAGE_ENTITY_NODE_TYPE;

AD_OBJECT_H(DoIPMessagePayloadEntityStatusResponse,
    DoIPMessageDef;
    uint16_t node_type;
    byte max_concurrent_connections;
    byte openned_connections;
    byte max_data_size;
)
#define DOIP_MESSAGE_ENTITY_STATUS_DEFAULT_MAX_DATA_SIZE 0xFF

AD_OBJECT_H(DoIPMessagePayloadRequestWithEID,
    DoIPMessageDef;
    Buffer * eid;
)
AD_OBJECT_H(DoIPMessagePayloadRequestWithVIN,
    DoIPMessageDef;
    Buffer * vin;
)
AD_OBJECT_H(DoIPMessagePayloadVehicleIdResponse,
    DoIPMessageDef;
    Buffer * vin;
    Buffer * addr;
    Buffer * eid;
    Buffer * gid;
    byte further_action_required;
    byte sync_status;
)
typedef enum {
    DOIP_MESSAGE_DIAG_POWER_MODE_READY = 0x00,
    DOIP_MESSAGE_DIAG_POWER_MODE_NOT_READY = 0x01,
    DOIP_MESSAGE_DIAG_POWER_MODE_UNSUPPORTED = 0x02,
    DOIP_MESSAGE_DIAG_POWER_MODE_RESERVED = 0x03 // to 0xFF
} DOIP_MESSAGE_DIAG_POWER_MODE_TYPE;

AD_OBJECT_H(DoIPMessagePayloadDiagPowerModeResponse,
    DoIPMessageDef;
    byte type;
)

#include "libautodiag/com/doip/device.h"
#include "libautodiag/com/doip/message/diag.h"

#endif