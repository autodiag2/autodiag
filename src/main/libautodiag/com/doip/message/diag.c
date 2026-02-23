#include "libautodiag/com/doip/message/diag.h"

object_DoIPMessage * doip_message_diag(Buffer * to, Buffer * from, Buffer * payload_data) {
    object_DoIPMessagePayloadDiag * payload = object_DoIPMessagePayloadDiag_new();
    payload->data = buffer_copy(payload_data);
    payload->src_addr = buffer_copy(from);
    payload->dst_addr = buffer_copy(to);
    object_DoIPMessage * msg = object_DoIPMessage_new();
    msg->payload = (DoIPMessageDef*)payload;
    msg->payload_type = DOIP_DIAGNOSTIC_MESSAGE;
    return msg;
}
object_DoIPMessagePayloadDiag *object_DoIPMessagePayloadDiag_new() {
    object_DoIPMessagePayloadDiag *payload = (object_DoIPMessagePayloadDiag*)malloc(sizeof(object_DoIPMessagePayloadDiag));
    if (!payload) return null;

    payload->src_addr = null;
    payload->dst_addr = null;
    payload->data = null;

    return payload;
}
object_DoIPMessagePayloadDiag* object_DoIPMessagePayloadDiag_assign(object_DoIPMessagePayloadDiag * to, object_DoIPMessagePayloadDiag * from) {
    assert(to != null);
    assert(from != null);
    buffer_assign(to->src_addr, from->src_addr);
    buffer_assign(to->dst_addr, from->dst_addr);
    buffer_assign(to->data, from->data);
    return to;
}
void object_DoIPMessagePayloadDiag_free(object_DoIPMessagePayloadDiag *payload) {
    if ( payload != null ) {
        buffer_free(payload->src_addr);
        payload->src_addr = null;
        buffer_free(payload->dst_addr);
        payload->dst_addr = null;
        buffer_free(payload->data);
        payload->data = null;
        free(payload);
    }
}

object_DoIPMessagePayloadDiagFeedback * object_DoIPMessagePayloadDiagFeedback_new() {
    object_DoIPMessagePayloadDiagFeedback * payload = (object_DoIPMessagePayloadDiagFeedback*)malloc(sizeof(object_DoIPMessagePayloadDiagFeedback));
    payload->src_addr = buffer_new();
    payload->dst_addr = buffer_new();
    payload->code = 0x00;
    payload->data = buffer_new();
    payload->type = DOIP_DIAGNOSTIC_MESSAGE_ACK;
    return payload;
}
void object_DoIPMessagePayloadDiagFeedback_free(object_DoIPMessagePayloadDiagFeedback * payload) {
    if ( payload != null ) {
        buffer_free(payload->src_addr);
        buffer_free(payload->dst_addr);
        buffer_free(payload->data);
        free(payload);
    }
}
object_DoIPMessagePayloadDiagFeedback * object_DoIPMessagePayloadDiagFeedback_assign(object_DoIPMessagePayloadDiagFeedback * to, object_DoIPMessagePayloadDiagFeedback * from) {
    assert(to != null);
    assert(from != null);
    buffer_assign(to->src_addr, from->src_addr);
    buffer_assign(to->dst_addr, from->dst_addr);
    to->code = from->code;
    buffer_assign(to->data, from->data);
    to->type = from->type;
    return to;
}

static object_DoIPMessage * message_feedback(Buffer * to, Buffer * from, Buffer * payload_data, byte code, uint16_t type) {
    object_DoIPMessage * msg = object_DoIPMessage_new();
    object_DoIPMessagePayloadDiagFeedback * payload = object_DoIPMessagePayloadDiagFeedback_new();
    buffer_assign(payload->src_addr, from);
    buffer_assign(payload->dst_addr, to);
    payload->code = code;
    buffer_assign(payload->data, payload_data);
    payload->type = type;
    msg->payload = (DoIPMessageDef*)payload;
    return msg;
}

object_DoIPMessage * doip_message_diag_feedback_ack(Buffer * to, Buffer * from, Buffer * payload_data, byte code) {
    return message_feedback(to, from, payload_data, code, DOIP_DIAGNOSTIC_MESSAGE_ACK);
}

object_DoIPMessage * doip_message_diag_feedback_nack(Buffer * to, Buffer * from, Buffer * payload_data, byte code) {
    return message_feedback(to, from, payload_data, code, DOIP_DIAGNOSTIC_MESSAGE_NACK);
}