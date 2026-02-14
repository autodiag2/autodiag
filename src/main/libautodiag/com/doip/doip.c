#include "libautodiag/com/doip/doip.h"

static int doip__be16(const byte *p) {
    return ((int)p[0] << 8) | (int)p[1];
}

static int doip__be32(const byte *p) {
    return ((int)p[0] << 24) | ((int)p[1] << 16) | ((int)p[2] << 8) | (int)p[3];
}

static object_DoIPPayload *doip__payload_new_empty() {
    object_DoIPPayload *pl = (object_DoIPPayload*)malloc(sizeof(object_DoIPPayload));
    pl->src_addr = NULL;
    pl->dst_addr = NULL;
    pl->data = NULL;
    return pl;
}

object_DoIPMessage * object_DoIPMessage_new() {
    object_DoIPMessage *msg = (object_DoIPMessage*)malloc(sizeof(object_DoIPMessage));
    msg->protocol_version = DOIP_PROTOCOL_VERSION_CURRENT;
    msg->inv_protocol_version = ~msg->protocol_version;
    msg->payload = doip__payload_new_empty();
    return msg;
}

object_DoIPMessage *doip_parse_message(const Buffer *in) {
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

    object_DoIPMessage *msg = object_DoIPMessage_new();
    if (!msg) return NULL;

    msg->protocol_version = ver;
    msg->inv_protocol_version = inv;
    msg->payload_type = (DoIpPayloadType)ptype;
    msg->payload_length = plen;

    object_DoIPPayload *pl = doip__payload_new_empty();
    if (!pl) {
        free(msg);
        return NULL;
    }

    if ((DoIpPayloadType)ptype == DOIP_DIAGNOSTIC_MESSAGE ||
        (DoIpPayloadType)ptype == DOIP_DIAGNOSTIC_MESSAGE_ACK ||
        (DoIpPayloadType)ptype == DOIP_DIAGNOSTIC_MESSAGE_NACK) {

        if (plen < 4) {
            free(pl);
            free(msg);
            return NULL;
        }

        pl->src_addr = buffer_slice((Buffer*)in, 8, 2);
        pl->dst_addr = buffer_slice((Buffer*)in, 10, 2);
        pl->data     = buffer_slice((Buffer*)in, 12, plen - 4);

    } else {
        pl->data = buffer_slice((Buffer*)in, 8, plen);
    }

    msg->payload = pl;
    return msg;
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

Buffer *doip_serialize_message(const object_DoIPMessage *msg) {
    if (!msg) return NULL;

    int plen = msg->payload_length;
    if (plen < 0) return NULL;

    int type = (int)msg->payload_type;

    int is_diag =
        (msg->payload_type == DOIP_DIAGNOSTIC_MESSAGE) ||
        (msg->payload_type == DOIP_DIAGNOSTIC_MESSAGE_ACK) ||
        (msg->payload_type == DOIP_DIAGNOSTIC_MESSAGE_NACK);

    int computed_plen = 0;

    if (is_diag) {
        if (!msg->payload) return NULL;
        if (!msg->payload->src_addr || msg->payload->src_addr->size != 2) return NULL;
        if (!msg->payload->dst_addr || msg->payload->dst_addr->size != 2) return NULL;
        computed_plen = 4 + (msg->payload->data ? msg->payload->data->size : 0);
    } else {
        computed_plen = (msg->payload && msg->payload->data) ? msg->payload->data->size : 0;
    }

    if (plen != computed_plen) plen = computed_plen;

    int out_sz = 8 + plen;
    if (out_sz < 8) return NULL;

    Buffer *out = (Buffer*)malloc(sizeof(Buffer));
    if (!out) return NULL;

    out->size = out_sz;
    out->buffer = (byte*)malloc((size_t)out_sz);
    if (!out->buffer) {
        free(out);
        return NULL;
    }

    byte *p = (byte*)out->buffer;

    p[0] = msg->protocol_version;
    p[1] = msg->inv_protocol_version;
    doip__put_be16(p + 2, type);
    doip__put_be32(p + 4, plen);

    if (is_diag) {
        memcpy(p + 8, msg->payload->src_addr->buffer, 2);
        memcpy(p + 10, msg->payload->dst_addr->buffer, 2);
        if (msg->payload->data && 0 < msg->payload->data->size) {
            memcpy(p + 12, msg->payload->data->buffer, (size_t)msg->payload->data->size);
        }
    } else {
        if (msg->payload && msg->payload->data && 0 < msg->payload->data->size) {
            memcpy(p + 8, msg->payload->data->buffer, (size_t)msg->payload->data->size);
        }
    }

    return out;
}
