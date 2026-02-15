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

object_DoIPDiagMessage *object_DoIPDiagMessage_new() {
    object_DoIPDiagMessage *msg = (object_DoIPDiagMessage*)malloc(sizeof(object_DoIPDiagMessage));
    if (!msg) return NULL;

    memset(msg, 0, sizeof(*msg));

    msg->protocol_version = DOIP_PROTOCOL_VERSION_CURRENT;
    msg->inv_protocol_version = (byte)~msg->protocol_version;
    msg->payload_type = DOIP_DIAGNOSTIC_MESSAGE;
    msg->payload_raw = NULL;

    msg->payload.src_addr = NULL;
    msg->payload.dst_addr = NULL;
    msg->payload.data = NULL;

    return msg;
}
object_DoIPDiagMessage* object_DoIPDiagMessage_assign(object_DoIPDiagMessage * to, object_DoIPDiagMessage * from) {
    assert(to != null);
    assert(from != null);
    to->protocol_version = from->protocol_version;
    to->inv_protocol_version = from->inv_protocol_version;
    to->payload_type = from->payload_type;
    if ( to->payload_raw != null ) {
        buffer_free(to->payload_raw);
    }
    to->payload_raw = buffer_copy(from->payload_raw);
    if ( to->payload.src_addr != null ) {
        buffer_free(to->payload.src_addr);
    }
    to->payload.src_addr = buffer_copy(from->payload.src_addr);
    if ( to->payload.dst_addr != null ) {
        buffer_free(to->payload.dst_addr);
    }
    to->payload.dst_addr = buffer_copy(from->payload.dst_addr);
    if ( to->payload.data != null ) {
        buffer_free(to->payload.data);
    }
    to->payload.data = buffer_copy(from->payload.data);
    return to;
}
void object_DoIPDiagMessage_free(object_DoIPDiagMessage *msg) {
    if ( msg != null ) {
        if ( msg->payload_raw != null ) {
            buffer_free(msg->payload_raw);
            msg->payload_raw = null;
        }
        if ( msg->payload.src_addr != null ) {
            buffer_free(msg->payload.src_addr);
            msg->payload.src_addr = null;
        }
        if ( msg->payload.dst_addr != null ) {
            buffer_free(msg->payload.dst_addr);
            msg->payload.dst_addr = null;
        }
        if ( msg->payload.data != null ) {
            buffer_free(msg->payload.data);
            msg->payload.data = null;
        }
        free(msg);
    }
}
object_DoIPMessage * doip_message_parse(const Buffer * in) {
    if (!in || !in->buffer) return NULL;
    if (in->size < 8) return NULL;

    const byte *p = (const byte*)in->buffer;

    byte ver = p[0];
    byte inv = p[1];
    if (((byte)(ver ^ inv)) != 0xFF) return NULL;

    int ptype = doip__be16(p + 2);
    int plen  = doip__be32(p + 4);
    if (plen < 0) return NULL;
    if (in->size < 8 + plen) return NULL;

    object_DoIPMessage *msg = (object_DoIPMessage*)malloc(sizeof(object_DoIPMessage));
    if (!msg) return NULL;

    memset(msg, 0, sizeof(*msg));

    msg->protocol_version = ver;
    msg->inv_protocol_version = inv;
    msg->payload_type = (DoIpPayloadType)ptype;
    msg->payload_raw = buffer_slice((Buffer*)in, 8, plen);

    return msg;
}
object_DoIPDiagMessage *doip_diag_message_parse(const Buffer *in) {
    if (!in || !in->buffer) return NULL;
    if (in->size < 8) return NULL;

    const byte *p = (const byte*)in->buffer;

    byte ver = p[0];
    byte inv = p[1];
    if (((byte)(ver ^ inv)) != 0xFF) return NULL;

    int ptype = doip__be16(p + 2);
    int plen  = doip__be32(p + 4);
    if (plen < 0) return NULL;
    if (in->size < 8 + plen) return NULL;

    if ((DoIpPayloadType)ptype != DOIP_DIAGNOSTIC_MESSAGE &&
        (DoIpPayloadType)ptype != DOIP_DIAGNOSTIC_MESSAGE_ACK &&
        (DoIpPayloadType)ptype != DOIP_DIAGNOSTIC_MESSAGE_NACK) {
        return NULL;
    }

    if (plen < 4) return NULL;

    object_DoIPDiagMessage *msg = object_DoIPDiagMessage_new();
    if (!msg) return NULL;

    msg->protocol_version = ver;
    msg->inv_protocol_version = inv;
    msg->payload_type = (DoIpPayloadType)ptype;

    msg->payload_raw = buffer_slice((Buffer*)in, 8, plen);

    msg->payload.src_addr = buffer_slice((Buffer*)in, 8, 2);
    msg->payload.dst_addr = buffer_slice((Buffer*)in, 10, 2);
    msg->payload.data     = buffer_slice((Buffer*)in, 12, plen - 4);

    return msg;
}
void doip_diag_message_dump(object_DoIPDiagMessage * msg) {
    assert(msg != null);
    if ( log_has_level(LOG_DEBUG) ) {
        log_msg(LOG_DEBUG, "diag message: {");
        log_msg(LOG_DEBUG, "    version: %02hhX", msg->protocol_version);
        log_msg(LOG_DEBUG, "    payload_type: %04X", msg->payload_type);
        log_msg(LOG_DEBUG, "    from: %s", buffer_to_hex_string(msg->payload.src_addr));
        log_msg(LOG_DEBUG, "    to: %s", buffer_to_hex_string(msg->payload.dst_addr));
        log_msg(LOG_DEBUG, "    payload: %s", buffer_to_hex_string(msg->payload.data));
        log_msg(LOG_DEBUG, "}");
    }
}
Buffer *doip_diag_message_serialize(const object_DoIPDiagMessage *msg) {
    if (!msg) return NULL;

    if (msg->payload_type != DOIP_DIAGNOSTIC_MESSAGE &&
        msg->payload_type != DOIP_DIAGNOSTIC_MESSAGE_ACK &&
        msg->payload_type != DOIP_DIAGNOSTIC_MESSAGE_NACK) {
        return NULL;
    }

    if (!msg->payload.src_addr || msg->payload.src_addr->size != 2) return NULL;
    if (!msg->payload.dst_addr || msg->payload.dst_addr->size != 2) return NULL;

    int dsz = msg->payload.data ? msg->payload.data->size : 0;
    if (dsz < 0) return NULL;

    int plen = 4 + dsz;
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

    memcpy(p + 8, msg->payload.src_addr->buffer, 2);
    memcpy(p + 10, msg->payload.dst_addr->buffer, 2);
    if (0 < dsz) memcpy(p + 12, msg->payload.data->buffer, (size_t)dsz);

    return out;
}
