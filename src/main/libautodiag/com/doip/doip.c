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
void doip_message_init(final object_DoIPMessage * msg, final DoIpPayloadType type) {
    assert(msg != null);
    msg->protocol_version = DOIP_PROTOCOL_VERSION_CURRENT;
    msg->inv_protocol_version = (byte)~msg->protocol_version;
    msg->payload_type = type;
    msg->payload_raw = null;
    msg->payload = null;
}
object_DoIPMessagePayloadDiag *object_DoIPMessagePayloadDiag_new() {
    object_DoIPMessagePayloadDiag *payload = (object_DoIPMessagePayloadDiag*)malloc(sizeof(object_DoIPMessagePayloadDiag));
    if (!payload) return null;

    payload->src_addr = null;
    payload->dst_addr = null;
    payload->data = null;

    return payload;
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
void object_DoIPMessage_free(object_DoIPMessage * msg) {
    if ( msg != null ) {
        if ( msg->payload_raw != null ) {
            buffer_free(msg->payload_raw);
            msg->payload_raw = null;
        }
        switch(msg->payload_type) {
            case DOIP_DIAGNOSTIC_MESSAGE: {
                object_DoIPMessagePayloadDiag_free(msg->payload);
            } break;
            default: {
                log_msg(LOG_DEBUG, "payload not implemented");
            }
        }
        msg->payload = null;
        free(msg);
    }
}
object_DoIPMessage * doip_diag_message(Buffer * to, Buffer * from, Buffer * payload_data) {
    object_DoIPMessagePayloadDiag * payload = object_DoIPMessagePayloadDiag_new();
    payload->data = buffer_copy(payload_data);
    payload->dst_addr = buffer_copy(to);
    payload->dst_addr = buffer_copy(from);
    object_DoIPMessage * msg = object_DoIPMessage_new();
    msg->payload = (DoIPMessageDef*)payload;
    msg->payload_type = DOIP_DIAGNOSTIC_MESSAGE;
    return msg;
}
object_DoIPMessagePayloadDiag* object_DoIPMessagePayloadDiag_assign(object_DoIPMessagePayloadDiag * to, object_DoIPMessagePayloadDiag * from) {
    assert(to != null);
    assert(from != null);
    object_DoIPMessage_assign((object_DoIPMessage*)to, (object_DoIPMessage*)from);
    if ( to->src_addr != null ) {
        buffer_free(to->src_addr);
    }
    to->src_addr = buffer_copy(from->src_addr);
    if ( to->dst_addr != null ) {
        buffer_free(to->dst_addr);
    }
    to->dst_addr = buffer_copy(from->dst_addr);
    if ( to->data != null ) {
        buffer_free(to->data);
    }
    to->data = buffer_copy(from->data);
    return to;
}
void object_DoIPMessagePayloadDiag_free(object_DoIPMessagePayloadDiag *payload) {
    if ( payload != null ) {
        if ( payload->src_addr != null ) {
            buffer_free(payload->src_addr);
            payload->src_addr = null;
        }
        if ( payload->dst_addr != null ) {
            buffer_free(payload->dst_addr);
            payload->dst_addr = null;
        }
        if ( payload->data != null ) {
            buffer_free(payload->data);
            payload->data = null;
        }
        free(payload);
    }
}
object_DoIPMessage * doip_message_parse(const Buffer * in) {
    assert(in != null);
    if (in->size < 8) return null;

    const byte *p = (const byte*)in->buffer;

    byte ver = p[0];
    byte inv = p[1];
    if (((byte)(ver ^ inv)) != 0xFF) return null;

    int ptype = doip__be16(p + 2);
    int plen  = doip__be32(p + 4);
    if (plen < 0) return null;
    if (in->size < 8 + plen) return null;

    object_DoIPMessage *msg = (object_DoIPMessage*)malloc(sizeof(object_DoIPMessage));
    if (!msg) return null;

    memset(msg, 0, sizeof(*msg));

    msg->protocol_version = ver;
    msg->inv_protocol_version = inv;
    msg->payload_type = (DoIpPayloadType)ptype;
    msg->payload_raw = buffer_slice((Buffer*)in, 8, plen);
    msg->payload = null;

    switch(ptype) {
        case DOIP_DIAGNOSTIC_MESSAGE: {
            object_DoIPMessagePayloadDiag * diag = (object_DoIPMessagePayloadDiag*)malloc(sizeof(object_DoIPMessagePayloadDiag));
            msg->payload = (DoIPMessageDef*)diag;
            assert(4 <= msg->payload_raw->size);
            diag->src_addr = buffer_slice(msg->payload_raw, 0, 2);
            diag->dst_addr = buffer_slice(msg->payload_raw, 2, 2);
            diag->data     = buffer_slice(msg->payload_raw, 4, msg->payload_raw->size - 4);
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
        log_msg(LOG_DEBUG, "    version: %02hhX", msg->protocol_version);
        log_msg(LOG_DEBUG, "    payload_type: %04X", msg->payload_type);
        log_msg(LOG_DEBUG, "}");
    }
}
Buffer *doip_message_serialize(const object_DoIPMessage *msg) {
    assert(msg != null);

    int plen = 0;
    switch(msg->payload_type) {
        case DOIP_DIAGNOSTIC_MESSAGE: {
            object_DoIPMessagePayloadDiag * payload = (object_DoIPMessagePayloadDiag*)msg->payload;
            if (!payload->src_addr || payload->src_addr->size != 2) return null;
            if (!payload->dst_addr || payload->dst_addr->size != 2) return null;

            int dsz = payload->data ? payload->data->size : 0;
            if (dsz < 0) return null;
            plen = 4 + dsz;
        } break;
        default: {
            log_msg(LOG_DEBUG, "Payload not implemented");
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

    switch(msg->payload_type) {
        case DOIP_DIAGNOSTIC_MESSAGE: {
            object_DoIPMessagePayloadDiag * payload = (object_DoIPMessagePayloadDiag*)msg->payload;
            memcpy(p + 8, payload->src_addr->buffer, 2);
            memcpy(p + 10, payload->dst_addr->buffer, 2);
            assert(payload->data->size <= (out_sz - 12));
            memcpy(p + 12, payload->data->buffer, payload->data->size);
        } break;
        default: {
            log_msg(LOG_DEBUG, "payload not implemented");
        } break;
    }

    return out;
}
