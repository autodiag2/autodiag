#include "libautodiag/com/doip/doip.h"

static int doip__be16(const byte *p) {
    return ((int)p[0] << 8) | (int)p[1];
}

static int doip__be32(const byte *p) {
    return ((int)p[0] << 24) | ((int)p[1] << 16) | ((int)p[2] << 8) | (int)p[3];
}

static void doip__put_be16(byte *p, int v) {
    p[0] = (byte)((v >> 8) & 0xFF);
    p[1] = (byte)(v & 0xFF);
}

static void doip__put_be32(byte *p, int v) {
    p[0] = (byte)((v >> 24) & 0xFF);
    p[1] = (byte)((v >> 16) & 0xFF);
    p[2] = (byte)((v >> 8) & 0xFF);
    p[3] = (byte)(v & 0xFF);
}
object_DoIPMessagePayloadEmpty * object_DoIPMessagePayloadEmpty_new() {
    object_DoIPMessagePayloadEmpty * payload = (object_DoIPMessagePayloadEmpty*)malloc(sizeof(object_DoIPMessagePayloadEmpty));
    if ( payload == null ) return null;
    memset(payload, 0, sizeof(*payload));
    return payload;
}
void object_DoIPMessagePayloadEmpty_free(object_DoIPMessagePayloadEmpty* payload) {
    if ( payload != null ) {
        memset(payload, 0, sizeof(*payload));
        free(payload);
    }
}
object_DoIPMessagePayloadEmpty * object_DoIPMessagePayloadEmpty_assign(object_DoIPMessagePayloadEmpty * to, object_DoIPMessagePayloadEmpty *from) {
    memcpy(to, from, sizeof(*to));
    return to;
}
void doip_message_init(final object_DoIPMessage * msg, final DoIpPayloadType type) {
    assert(msg != null);
    msg->protocol_version = DOIP_PROTOCOL_VERSION_CURRENT;
    msg->inv_protocol_version = (byte)~msg->protocol_version;
    msg->payload_type = type;
    msg->payload_raw = null;
    msg->payload = null;
    switch(type) {
        case DOIP_DIAGNOSTIC_MESSAGE: {
            msg->payload = (DoIPMessageDef*)object_DoIPMessagePayloadDiag_new();
        } break;
        case DOIP_ROUTING_ACTIVATION_REQUEST: {
            msg->payload = (DoIPMessageDef*)object_DoIPMessagePayloadEmpty_new();
        } break;
        case DOIP_ENTITY_STATUS_REQUEST: {
            msg->payload = (DoIPMessageDef*)object_DoIPMessagePayloadEmpty_new();
        } break;
        case DOIP_ENTITY_STATUS_RESPONSE: {
            msg->payload = (DoIPMessageDef*)object_DoIPMessagePayloadEntityStatusResponse_new();
        } break;
        case DOIP_ALIVE_CHECK_REQUEST:
        case DOIP_ALIVE_CHECK_RESPONSE: {
            msg->payload = (DoIPMessageDef*)object_DoIPMessagePayloadAliveCheck_new();
        } break;
        case DOIP_DIAGNOSTIC_MESSAGE_ACK:
        case DOIP_DIAGNOSTIC_MESSAGE_NACK: {
            object_DoIPMessagePayloadDiagFeedback * payload = object_DoIPMessagePayloadDiagFeedback_new();
            payload->type = type;
            msg->payload = (DoIPMessageDef*)payload;
        } break;
        case DOIP_ROUTING_ACTIVATION_RESPONSE: {
            msg->payload = (DoIPMessageDef*)object_DoIPMessagePayloadRoutineActivationResponse_new();
        } break;
        default: {
            log_msg(LOG_DEBUG, "init: payload not implemented 0x%04X", type);
        } break;
    }
}
object_DoIPMessagePayloadRoutineActivationRequest * object_DoIPMessagePayloadRoutineActivationRequest_new() {
    object_DoIPMessagePayloadRoutineActivationRequest * payload = (object_DoIPMessagePayloadRoutineActivationRequest*)malloc(sizeof(object_DoIPMessagePayloadRoutineActivationRequest));
    if ( payload == null ) return null;
    memset(payload, 0, sizeof(*payload));
    payload->activation_type = DOIP_MESSAGE_RAR_TYPE_DEFAULT;
    return payload;
}
void object_DoIPMessagePayloadRoutineActivationRequest_free(object_DoIPMessagePayloadRoutineActivationRequest* payload) {
    if ( payload != null ) {
        memset(payload, 0, sizeof(*payload));
        free(payload);
    }
}
object_DoIPMessagePayloadRoutineActivationRequest * object_DoIPMessagePayloadRoutineActivationRequest_assign(object_DoIPMessagePayloadRoutineActivationRequest * to, object_DoIPMessagePayloadRoutineActivationRequest *from) {
    memcpy(to, from, sizeof(*to));
    return to;
}
object_DoIPMessage * object_DoIPMessage_assign(object_DoIPMessage * to, object_DoIPMessage * from) {
    to->protocol_version = from->protocol_version;
    to->inv_protocol_version = from->inv_protocol_version;
    to->payload_type = from->payload_type;
    if ( to->payload_raw != null ) {
        buffer_free(to->payload_raw);
    }
    to->payload_raw = buffer_copy(from->payload_raw);
    return to;
}
object_DoIPMessage * object_DoIPMessage_new() {
    object_DoIPMessage * msg = (object_DoIPMessage*)malloc(sizeof(object_DoIPMessage));
    doip_message_init(msg, 0x00);
    return msg;
}
object_DoIPMessage * doip_message_new(DoIpPayloadType type) {
    object_DoIPMessage * msg = object_DoIPMessage_new();
    doip_message_init(msg, type);
    return msg;
}
void object_DoIPMessage_free(object_DoIPMessage * msg) {
    if ( msg != null ) {
        if ( msg->payload_raw != null ) {
            buffer_free(msg->payload_raw);
            msg->payload_raw = null;
        }
        switch(msg->payload_type) {
            case DOIP_ALIVE_CHECK_REQUEST:
            case DOIP_ALIVE_CHECK_RESPONSE: {
                object_DoIPMessagePayloadAliveCheck_free((object_DoIPMessagePayloadAliveCheck*)msg->payload);
            } break;
            case DOIP_DIAGNOSTIC_MESSAGE: {
                object_DoIPMessagePayloadDiag_free((object_DoIPMessagePayloadDiag*)msg->payload);
            } break;
            case DOIP_ROUTING_ACTIVATION_REQUEST: {
                object_DoIPMessagePayloadRoutineActivationRequest_free((object_DoIPMessagePayloadRoutineActivationRequest*)msg->payload);
            } break;
            case DOIP_ENTITY_STATUS_REQUEST: {
                object_DoIPMessagePayloadEmpty_free((object_DoIPMessagePayloadEmpty*)msg->payload);
            } break;
            case DOIP_ENTITY_STATUS_RESPONSE: {
                object_DoIPMessagePayloadEntityStatusResponse_free((object_DoIPMessagePayloadEntityStatusResponse*)msg->payload);
            } break;
            case DOIP_DIAGNOSTIC_MESSAGE_ACK:
            case DOIP_DIAGNOSTIC_MESSAGE_NACK: {
                object_DoIPMessagePayloadDiagFeedback_free((object_DoIPMessagePayloadDiagFeedback*)msg->payload);
            } break;
            case DOIP_ROUTING_ACTIVATION_RESPONSE: {
                object_DoIPMessagePayloadRoutineActivationResponse_free((object_DoIPMessagePayloadRoutineActivationResponse*)msg->payload);
            } break;
            default: {
                log_msg(LOG_DEBUG, "free: payload not implemented %04X", msg->payload_type);
            }
        }
        msg->payload = null;
        free(msg);
    }
}
object_DoIPMessage * doip_message_parse(const Buffer * in) {
    assert(in != null);
    if (in->size < 8) {
        log_msg(LOG_DEBUG, "parsing:length too short");
        return null;
    }

    const byte *p = (const byte*)in->buffer;

    byte ver = p[0];
    byte inv = p[1];
    if (((byte)(ver ^ inv)) != 0xFF) {
        log_msg(LOG_DEBUG, "parsing:checksum incorrect");
        return null;
    }

    uint16_t ptype = doip__be16(p + 2);
    uint32_t plen  = doip__be32(p + 4);
    if (plen < 0) return null;
    if (in->size < 8 + plen) return null;

    object_DoIPMessage *msg = object_DoIPMessage_new();
    if (!msg) {
        log_msg(LOG_DEBUG, "parsing:allocation failed");
        return null;
    }

    memset(msg, 0, sizeof(*msg));

    msg->protocol_version = ver;
    msg->inv_protocol_version = inv;
    msg->payload_type = (DoIpPayloadType)ptype;
    msg->payload_raw = buffer_slice((Buffer*)in, 8, plen);
    msg->payload = null;

    log_msg(LOG_DEBUG, "parsing:found payload type 0x%04X", ptype);
    switch(ptype) {
        case DOIP_DIAGNOSTIC_MESSAGE: {
            object_DoIPMessagePayloadDiag * payload = object_DoIPMessagePayloadDiag_new();
            msg->payload = (DoIPMessageDef*)payload;
            assert(4 <= msg->payload_raw->size);
            payload->src_addr = buffer_slice(msg->payload_raw, 0, 2);
            payload->dst_addr = buffer_slice(msg->payload_raw, 2, 2);
            payload->data     = buffer_slice(msg->payload_raw, 4, msg->payload_raw->size - 4);
        } break;
        case DOIP_ALIVE_CHECK_REQUEST: {
            object_DoIPMessagePayloadAliveCheck * payload = object_DoIPMessagePayloadAliveCheck_new();
            buffer_recycle(payload->src_addr);
        } break;
        case DOIP_ALIVE_CHECK_RESPONSE: {
            object_DoIPMessagePayloadAliveCheck * payload = object_DoIPMessagePayloadAliveCheck_new();
            buffer_recycle(payload->src_addr);
            buffer_slice_append(payload->src_addr, msg->payload_raw, 0, 2);
        } break;
        case DOIP_DIAGNOSTIC_MESSAGE_NACK:
        case DOIP_DIAGNOSTIC_MESSAGE_ACK: {
            object_DoIPMessagePayloadDiagFeedback * payload = object_DoIPMessagePayloadDiagFeedback_new();
            msg->payload = (DoIPMessageDef*)payload;
            assert(5 <= msg->payload_raw->size);
            payload->src_addr = buffer_slice(msg->payload_raw, 0, 2);
            payload->dst_addr = buffer_slice(msg->payload_raw, 2, 2);
            payload->code = msg->payload_raw->buffer[4];
            if (5 < msg->payload_raw->size) {
                payload->data = buffer_slice(msg->payload_raw, 5, msg->payload_raw->size - 5);
            }
            payload->type = ptype;
        } break;
        case DOIP_ENTITY_STATUS_REQUEST: {
            object_DoIPMessagePayloadEmpty * payload = object_DoIPMessagePayloadEmpty_new();
            msg->payload = (DoIPMessageDef*)payload;
        } break;
        case DOIP_ENTITY_STATUS_RESPONSE: {
            object_DoIPMessagePayloadEntityStatusResponse * payload = object_DoIPMessagePayloadEntityStatusResponse_new();
            msg->payload = (DoIPMessageDef*)payload;
            assert(7 <= msg->payload_raw->size);
            payload->node_type = doip__be16(msg->payload_raw->buffer);
            payload->max_concurrent_connections = msg->payload_raw->buffer[2];
            payload->openned_connections = msg->payload_raw->buffer[3];
            payload->max_data_size = msg->payload_raw->buffer[4];
        } break;
        case DOIP_ROUTING_ACTIVATION_REQUEST: {
            object_DoIPMessagePayloadRoutineActivationRequest * payload = object_DoIPMessagePayloadRoutineActivationRequest_new();
            msg->payload = (DoIPMessageDef*)payload;
            assert(3 <= msg->payload_raw->size);
            memcpy(payload->src_addr, msg->payload_raw->buffer, 2);
            payload->activation_type = msg->payload_raw->buffer[2];
            if ( 7 <= msg->payload_raw->size ) {
                memcpy(payload->iso_reserved, msg->payload_raw->buffer + 3, 4);
            }
            if ( 11 <= msg->payload_raw->size ) {
                memcpy(payload->vendor_reserved, msg->payload_raw->buffer + 7, 4);
            }
        } break;
        case DOIP_ROUTING_ACTIVATION_RESPONSE: {
            object_DoIPMessagePayloadRoutineActivationResponse * payload = object_DoIPMessagePayloadRoutineActivationResponse_new();
            msg->payload = (DoIPMessageDef*)payload;
            assert(13 <= msg->payload_raw->size);
            buffer_memcpy(payload->tester, msg->payload_raw->buffer, 2);
            buffer_memcpy(payload->ecu, msg->payload_raw->buffer+2, 2);
            payload->code = msg->payload_raw->buffer[4];
            buffer_memcpy(payload->iso_reserved, msg->payload_raw->buffer+5, 4);
            buffer_memcpy(payload->oem_reserved, msg->payload_raw->buffer+9, 4);
        } break;
        default: {
            log_msg(LOG_DEBUG, "Parsing of payload type 0x%04X not implemented", ptype);
        } break;
    }

    return msg;
}

void doip_message_dump(object_DoIPMessage * msg) {
    assert(msg != null);
    if ( log_has_level(LOG_DEBUG) ) {
        log_msg(LOG_DEBUG, "diag message: {");
        log_msg(LOG_DEBUG, "    version: 0x%02hhX", msg->protocol_version);
        log_msg(LOG_DEBUG, "    payload_type: 0x%04X", msg->payload_type);
        switch(msg->payload_type) {
            case DOIP_DIAGNOSTIC_MESSAGE: {
                object_DoIPMessagePayloadDiag * payload = (object_DoIPMessagePayloadDiag*)msg->payload;
                log_msg(LOG_DEBUG, "    src_addr: %s", buffer_to_hex_string(payload->src_addr));
                log_msg(LOG_DEBUG, "    dst_addr: %s", buffer_to_hex_string(payload->dst_addr));
                log_msg(LOG_DEBUG, "    data: %s", buffer_to_hex_string(payload->data));
            } break;
            case DOIP_ALIVE_CHECK_REQUEST:
            case DOIP_ALIVE_CHECK_RESPONSE: {
                object_DoIPMessagePayloadAliveCheck * payload = (object_DoIPMessagePayloadAliveCheck*)msg->payload;
                log_msg(LOG_DEBUG, "    src_addr: 0x%04X", buffer_to_hex_string(payload->src_addr));
            } break;
            case DOIP_ROUTING_ACTIVATION_REQUEST: {
                object_DoIPMessagePayloadRoutineActivationRequest * payload = (object_DoIPMessagePayloadRoutineActivationRequest*)msg->payload;
                log_msg(LOG_DEBUG, "    src_addr: 0x%02X%02X", payload->src_addr[0], payload->src_addr[1]);
                log_msg(LOG_DEBUG, "    activation_type: 0x%02x", payload->activation_type);
                log_msg(LOG_DEBUG, "    iso_reserved: 0x%02x%02x%02x%02x", payload->iso_reserved[0], payload->iso_reserved[1], payload->iso_reserved[2], payload->iso_reserved[3]);
                log_msg(LOG_DEBUG, "    vendor_reserved: 0x%02x%02x%02x%02x", payload->vendor_reserved[0], payload->vendor_reserved[1], payload->vendor_reserved[2], payload->vendor_reserved[3]);
            } break;
            case DOIP_ENTITY_STATUS_REQUEST: {
                log_msg(LOG_DEBUG, "    no payload");
            } break;
            case DOIP_ENTITY_STATUS_RESPONSE: {
                object_DoIPMessagePayloadEntityStatusResponse * payload = (object_DoIPMessagePayloadEntityStatusResponse*)msg->payload;
                log_msg(LOG_DEBUG, "    node_type: 0x%04X", payload->node_type);
                log_msg(LOG_DEBUG, "    max_concurrent_connections: %d", payload->max_concurrent_connections);
                log_msg(LOG_DEBUG, "    openned_connections: %d", payload->openned_connections);
                log_msg(LOG_DEBUG, "    max_data_size: %d", payload->max_data_size);
            } break;
            case DOIP_DIAGNOSTIC_MESSAGE_NACK:
            case DOIP_DIAGNOSTIC_MESSAGE_ACK: {
                object_DoIPMessagePayloadDiagFeedback * payload = (object_DoIPMessagePayloadDiagFeedback*)msg->payload;
                log_msg(LOG_DEBUG, "    type: 0x%08X", payload->type);
                log_msg(LOG_DEBUG, "    src_addr: %s", buffer_to_hex_string(payload->src_addr));
                log_msg(LOG_DEBUG, "    dst_addr: %s", buffer_to_hex_string(payload->dst_addr));
                log_msg(LOG_DEBUG, "    code: 0x%02X", payload->code);
                if (payload->data) {
                    log_msg(LOG_DEBUG, "    data: %s", buffer_to_hex_string(payload->data));
                }
            } break;
            case DOIP_ROUTING_ACTIVATION_RESPONSE: {
                object_DoIPMessagePayloadRoutineActivationResponse * payload = (object_DoIPMessagePayloadRoutineActivationResponse*)msg->payload;
                log_msg(LOG_DEBUG, "    tester: 0x%s", buffer_to_hex_string(payload->tester));
                log_msg(LOG_DEBUG, "    ecu: 0x%s", buffer_to_hex_string(payload->ecu));
                log_msg(LOG_DEBUG, "    code: 0x%02X", payload->code);
                log_msg(LOG_DEBUG, "    iso_reserved: 0x%s", buffer_to_hex_string(payload->iso_reserved));
                log_msg(LOG_DEBUG, "    oem_reserved: 0x%s", buffer_to_hex_string(payload->oem_reserved));
            } break;
            default: {

            } break;
        }
        log_msg(LOG_DEBUG, "}");
    }
}
Buffer *doip_message_serialize(const object_DoIPMessage *msg) {
    assert(msg != null);

    int plen = 0;
    switch(msg->payload_type) {
        case DOIP_DIAGNOSTIC_MESSAGE: {
            object_DoIPMessagePayloadDiag * payload = (object_DoIPMessagePayloadDiag*)msg->payload;
            if (!payload->src_addr || payload->src_addr->size != 2) {
                log_msg(LOG_ERROR, "src addr incorrect");
                return null;
            }
            if (!payload->dst_addr || payload->dst_addr->size != 2) {
                log_msg(LOG_ERROR, "dst addr incorrect");
                return null;
            }

            int dsz = payload->data ? payload->data->size : 0;
            if (dsz < 0) {
                log_msg(LOG_ERROR, "dsz incorrect");
                return null;
            }
            plen = 4 + dsz;
        } break;
        case DOIP_ALIVE_CHECK_REQUEST:
        case DOIP_ALIVE_CHECK_RESPONSE: {
            object_DoIPMessagePayloadAliveCheck * payload = (object_DoIPMessagePayloadAliveCheck*)msg->payload;
            plen = payload->src_addr->size;
        } break;
        case DOIP_ROUTING_ACTIVATION_REQUEST: {
            object_DoIPMessagePayloadRoutineActivationRequest * payload = (object_DoIPMessagePayloadRoutineActivationRequest*)msg->payload;
            plen = 2 + 1 + 4 + 4;
        } break;
        case DOIP_ENTITY_STATUS_REQUEST: {
            plen = 0;
        } break;
        case DOIP_ENTITY_STATUS_RESPONSE: {
            object_DoIPMessagePayloadEntityStatusResponse * payload = (object_DoIPMessagePayloadEntityStatusResponse*)msg->payload;
            plen = 2 + 1 + 1 + 1;
        } break;
        case DOIP_DIAGNOSTIC_MESSAGE_ACK:
        case DOIP_DIAGNOSTIC_MESSAGE_NACK: {
            object_DoIPMessagePayloadDiagFeedback * payload = (object_DoIPMessagePayloadDiagFeedback*)msg->payload;
            if (!payload->src_addr || payload->src_addr->size != 2) {
                log_msg(LOG_ERROR, "src addr incorrect");
                return null;
            }
            if (!payload->dst_addr || payload->dst_addr->size != 2) {
                log_msg(LOG_ERROR, "dst addr incorrect");
                return null;
            }
            int dsz = payload->data ? payload->data->size : 0;
            if (dsz < 0) {
                log_msg(LOG_ERROR, "dsz incorrect");
                return null;
            }
            plen = 5 + dsz;
        } break;
        case DOIP_ROUTING_ACTIVATION_RESPONSE: {
            object_DoIPMessagePayloadRoutineActivationResponse * payload = (object_DoIPMessagePayloadRoutineActivationResponse*)msg->payload;
            plen = 2 + 2 + 1 + 4 + 4;
        } break;
        default: {
            log_msg(LOG_DEBUG, "serialize2: Payload not implemented 0x%04X", msg->payload_type);
        } break;
    }
    int out_sz = 8 + plen;

    Buffer *out = buffer_new();

    buffer_ensure_capacity(out, out_sz);
    out->size = out_sz;

    byte *p = (byte*)out->buffer;

    byte ver = msg->protocol_version ? msg->protocol_version : DOIP_PROTOCOL_VERSION_CURRENT;
    byte inv = (byte)~ver;

    p[0] = ver;
    p[1] = inv;
    doip__put_be16(p + 2, (int)msg->payload_type);
    doip__put_be32(p + 4, plen);

    byte * payload_start = p + 8;

    switch(msg->payload_type) {
        case DOIP_DIAGNOSTIC_MESSAGE: {
            object_DoIPMessagePayloadDiag * payload = (object_DoIPMessagePayloadDiag*)msg->payload;
            memcpy(payload_start, payload->src_addr->buffer, 2);
            memcpy(payload_start + 2, payload->dst_addr->buffer, 2);
            assert(payload->data->size <= (out_sz - 12));
            memcpy(payload_start + 4, payload->data->buffer, payload->data->size);
        } break;
        case DOIP_ALIVE_CHECK_REQUEST:
        case DOIP_ALIVE_CHECK_RESPONSE: {
            object_DoIPMessagePayloadAliveCheck * payload = (object_DoIPMessagePayloadAliveCheck*)msg->payload;
            memcpy(payload_start, payload->src_addr->buffer, payload->src_addr->size);
        } break;
        case DOIP_ROUTING_ACTIVATION_REQUEST: {
            object_DoIPMessagePayloadRoutineActivationRequest * payload = (object_DoIPMessagePayloadRoutineActivationRequest*)msg->payload;
            memcpy(payload_start, payload->src_addr, 2);
            payload_start[2] = payload->activation_type;
            memcpy(payload_start + 3, payload->iso_reserved, 4);
            memcpy(payload_start + 7, payload->vendor_reserved, 4);
        } break;
        case DOIP_ENTITY_STATUS_REQUEST: {
            // no payload
        } break;
        case DOIP_ENTITY_STATUS_RESPONSE: {
            object_DoIPMessagePayloadEntityStatusResponse * payload = (object_DoIPMessagePayloadEntityStatusResponse*)msg->payload;
            doip__put_be16(payload_start, payload->node_type);
            payload_start[2] = payload->max_concurrent_connections;
            payload_start[3] = payload->openned_connections;
            payload_start[4] = payload->max_data_size;
        } break;
        case DOIP_DIAGNOSTIC_MESSAGE_ACK:
        case DOIP_DIAGNOSTIC_MESSAGE_NACK: {
            object_DoIPMessagePayloadDiagFeedback * payload = (object_DoIPMessagePayloadDiagFeedback*)msg->payload;
            memcpy(payload_start, payload->src_addr->buffer, 2);
            memcpy(payload_start + 2, payload->dst_addr->buffer, 2);
            payload_start[4] = payload->code;
            if (payload->data) {
                assert(payload->data->size <= (out_sz - 13));
                memcpy(payload_start + 5, payload->data->buffer, payload->data->size);
            }
        } break;
        case DOIP_ROUTING_ACTIVATION_RESPONSE: {
            object_DoIPMessagePayloadRoutineActivationResponse * payload = (object_DoIPMessagePayloadRoutineActivationResponse*)msg->payload;
            assert(2 <= payload->tester->size);
            memcpy(payload_start, payload->tester->buffer, 2);
            assert(2 <= payload->ecu->size);
            memcpy(payload_start + 2, payload->ecu->buffer, 2);
            assert(payload->code != DOIP_MESSAGE_RARES_CODE_UNKNOWN);
            payload_start[4] = payload->code;
            assert(4 <= payload->iso_reserved->size);
            memcpy(payload_start + 5, payload->iso_reserved->buffer, 4);
            assert(4 <= payload->oem_reserved->size);
            memcpy(payload_start + 9, payload->oem_reserved->buffer, 4);
        } break;
        default: {
            log_msg(LOG_DEBUG, "serialize: payload not implemented 0x%04X", msg->payload_type);
        } break;
    }
    assert(out != null);
    return out;
}

char *doip_message_rares_code_to_string(unsigned code) {
    switch (code) {
        case DOIP_MESSAGE_RARES_CODE_UNKNOWN_SOURCE_ADDRESS:
            return strdup("Routing activation denied due to unknown source address");
        case DOIP_MESSAGE_RARES_CODE_ALL_SOCKETS_REGISTERED_AND_ACTIVE:
            return strdup("Routing activation denied because all TCP_DATA sockets are active");
        case DOIP_MESSAGE_RARES_CODE_DIFFERENT_SA_ON_ALREADY_ACTIVATED_SOCKET:
            return strdup("Routing activation denied: different source address on active socket");
        case DOIP_MESSAGE_RARES_CODE_SA_ALREADY_ACTIVE_ON_DIFFERENT_SOCKET:
            return strdup("Routing activation denied: source address already active elsewhere");
        case DOIP_MESSAGE_RARES_CODE_MISSING_AUTHENTICATION:
            return strdup("Routing activation denied due to missing authentication");
        case DOIP_MESSAGE_RARES_CODE_REJECTED_CONFIRMATION:
            return strdup("Routing activation denied due to rejected confirmation");
        case DOIP_MESSAGE_RARES_CODE_UNSUPPORTED_ROUTING_ACTIVATION_TYPE:
            return strdup("Routing activation denied: unsupported activation type");
        case DOIP_MESSAGE_RARES_CODE_SUCCESS:
            return strdup("Routing successfully activated");
        case DOIP_MESSAGE_RARES_CODE_CONFIRMATION_REQUIRED:
            return strdup("Routing activation pending confirmation");
        case DOIP_MESSAGE_RARES_CODE_RESERVED_ISO_13400_LOW:
        case DOIP_MESSAGE_RARES_CODE_RESERVED_ISO_13400_MID:
        case DOIP_MESSAGE_RARES_CODE_RESERVED_ISO_13400_HIGH:
            return strdup("Reserved ISO 13400 range");
        case DOIP_MESSAGE_RARES_CODE_OEM_SPECIFIC:
            return strdup("Vehicle manufacturer specific");
        default:
            return strdup("Unknown routing activation response code");
    }
}

DOIP_MESSAGE_RARES_CODE doip_message_rares_n_to_code(unsigned rares_n) {
    switch (rares_n) {
        case 0x00: return DOIP_MESSAGE_RARES_CODE_UNKNOWN_SOURCE_ADDRESS;
        case 0x01: return DOIP_MESSAGE_RARES_CODE_ALL_SOCKETS_REGISTERED_AND_ACTIVE;
        case 0x02: return DOIP_MESSAGE_RARES_CODE_DIFFERENT_SA_ON_ALREADY_ACTIVATED_SOCKET;
        case 0x03: return DOIP_MESSAGE_RARES_CODE_SA_ALREADY_ACTIVE_ON_DIFFERENT_SOCKET;
        case 0x04: return DOIP_MESSAGE_RARES_CODE_MISSING_AUTHENTICATION;
        case 0x05: return DOIP_MESSAGE_RARES_CODE_REJECTED_CONFIRMATION;
        case 0x06: return DOIP_MESSAGE_RARES_CODE_UNSUPPORTED_ROUTING_ACTIVATION_TYPE;
        case 0x10: return DOIP_MESSAGE_RARES_CODE_SUCCESS;
        case 0x11: return DOIP_MESSAGE_RARES_CODE_CONFIRMATION_REQUIRED;
    }

    if (0x07 <= rares_n && rares_n <= 0x0F) return DOIP_MESSAGE_RARES_CODE_RESERVED_ISO_13400_LOW;
    if (0x12 <= rares_n && rares_n <= 0xDF) return DOIP_MESSAGE_RARES_CODE_RESERVED_ISO_13400_MID;
    if (0xE0 <= rares_n && rares_n <= 0xFE) return DOIP_MESSAGE_RARES_CODE_OEM_SPECIFIC;
    if (rares_n == 0xFF) return DOIP_MESSAGE_RARES_CODE_RESERVED_ISO_13400_HIGH;

    return DOIP_MESSAGE_RARES_CODE_UNKNOWN;
}

object_DoIPMessagePayloadRoutineActivationResponse * object_DoIPMessagePayloadRoutineActivationResponse_assign(object_DoIPMessagePayloadRoutineActivationResponse * to, object_DoIPMessagePayloadRoutineActivationResponse * from) {
    to->tester = buffer_copy(from->tester);
    to->ecu = buffer_copy(from->ecu);
    to->code = from->code;
    buffer_assign(to->iso_reserved, from->iso_reserved);
    buffer_assign(to->oem_reserved, from->oem_reserved);
    return to;
}

object_DoIPMessagePayloadRoutineActivationResponse* object_DoIPMessagePayloadRoutineActivationResponse_new() {
    object_DoIPMessagePayloadRoutineActivationResponse * r = (object_DoIPMessagePayloadRoutineActivationResponse*)malloc(sizeof(object_DoIPMessagePayloadRoutineActivationResponse));
    r->tester = buffer_new();
    r->ecu = buffer_new();
    r->code = DOIP_MESSAGE_RARES_CODE_UNKNOWN;
    r->iso_reserved = buffer_new();
    r->oem_reserved = buffer_new();
    return r;
}
void object_DoIPMessagePayloadRoutineActivationResponse_free(object_DoIPMessagePayloadRoutineActivationResponse * r) {
    if ( r != null ) {
        buffer_free(r->tester);
        buffer_free(r->ecu);
        buffer_free(r->iso_reserved);
        buffer_free(r->oem_reserved);
        free(r);
    }
}

object_DoIPMessagePayloadAliveCheck * object_DoIPMessagePayloadAliveCheck_new() {
    object_DoIPMessagePayloadAliveCheck * r = (object_DoIPMessagePayloadAliveCheck*)malloc(sizeof(object_DoIPMessagePayloadAliveCheck));
    r->src_addr = buffer_new();
    return r;
}

void object_DoIPMessagePayloadAliveCheck_free(object_DoIPMessagePayloadAliveCheck * payload) {
    if ( payload != null ) {
        buffer_free(payload->src_addr);
        free(payload);
    }
}

object_DoIPMessagePayloadAliveCheck * object_DoIPMessagePayloadAliveCheck_assign(object_DoIPMessagePayloadAliveCheck * to, object_DoIPMessagePayloadAliveCheck * from) {
    buffer_assign(to->src_addr, from->src_addr);
    return to;
}
object_DoIPMessagePayloadEntityStatusResponse * object_DoIPMessagePayloadEntityStatusResponse_new() {
    object_DoIPMessagePayloadEntityStatusResponse * r = (object_DoIPMessagePayloadEntityStatusResponse*)malloc(sizeof(object_DoIPMessagePayloadEntityStatusResponse));
    r->node_type = DOIP_MESSAGE_ENTITY_NODE_TYPE_UNSET;
    r->openned_connections = 0;
    r->max_concurrent_connections = 0;
    r->max_data_size = DOIP_MESSAGE_ENTITY_STATUS_DEFAULT_MAX_DATA_SIZE;
    return r;
}
void object_DoIPMessagePayloadEntityStatusResponse_free(object_DoIPMessagePayloadEntityStatusResponse * payload) {
    if ( payload != null ) {
        free(payload);
    }
}
object_DoIPMessagePayloadEntityStatusResponse * object_DoIPMessagePayloadEntityStatusResponse_assign(object_DoIPMessagePayloadEntityStatusResponse * to, object_DoIPMessagePayloadEntityStatusResponse * from) {
    to->node_type = from->node_type;
    to->openned_connections = from->openned_connections;
    to->max_concurrent_connections = from->max_concurrent_connections;
    to->max_data_size = from->max_data_size;
    return to;
}