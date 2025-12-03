#include "libautodiag/com/serial/elm/elm327/iso15765.h"

bool elm327_protocol_is_can_11_bits_id(final ELM327_PROTO proto) {
    switch(proto) {
        case ELM327_PROTO_ISO_15765_4_CAN_1:
        case ELM327_PROTO_ISO_15765_4_CAN_3:
        case ELM327_PROTO_USER1_CAN:
        case ELM327_PROTO_USER2_CAN:
            return true;
        default:
            return false;
    }
    return false;
}
bool elm327_is_can_11_bits_id(final ELM327Device* elm327) {
    return elm327_protocol_is_can_11_bits_id(elm327->protocol);
}

bool elm327_protocol_is_can_29_bits_id(final ELM327_PROTO proto) {
    switch(proto) {
        case ELM327_PROTO_ISO_15765_4_CAN_2:
        case ELM327_PROTO_ISO_15765_4_CAN_4:
        case ELM327_PROTO_SAEJ1939:
            return true;
        default:
            return false;
    }
    return false;
}
bool elm327_is_can_29_bits_id(final ELM327Device* elm327) {
    return elm327_protocol_is_can_29_bits_id(elm327->protocol);
}

bool elm327_iso15765_parse_response(final ELM327Device* elm327, final Vehicle* vehicle) {
    assert(elm327->eol != null);
    log_msg(LOG_WARNING, "Assuming auto formatting on");
    final int id_sz_chars;
    if ( elm327_is_can_11_bits_id(elm327) ) {
        id_sz_chars = 3;
    } else if ( elm327_is_can_29_bits_id(elm327) ) {
        id_sz_chars = 8;
    } else {
        log_msg(LOG_ERROR, "Not an obd can interface (wrong protocol)");
        return false;
    }
    final byte id_ascii[id_sz_chars+1];
    id_ascii[id_sz_chars] = 0;

    final ELMDevice* elm = (ELMDevice*)elm327;
    final list_Iso15765Conversation* conversations = list_Iso15765Conversation_new();
    SERIAL_BUFFER_ITERATE(elm,ELM_ISO15765_PARSE_RESPONSE_ITERATOR)
    list_Iso15765Conversation_free(conversations);
    return true;
}
