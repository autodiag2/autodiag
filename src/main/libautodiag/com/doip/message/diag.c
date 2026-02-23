#include "libautodiag/com/doip/message/diag.h"

object_DoIPMessage * doip_diag_message(Buffer * to, Buffer * from, Buffer * payload_data) {
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