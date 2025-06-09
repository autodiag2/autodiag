#include "libautodiag/com/serial/elm/elm329/iso15765.h"


bool elm329_is_can_11_bits_id(final ELM329Device* elm329) {
    switch(elm329->protocol) {
        case ELM329_PROTO_ISO_15765_4_CAN_1:
        case ELM329_PROTO_ISO_15765_4_CAN_3:
        case ELM329_PROTO_USER1_CAN:
        case ELM329_PROTO_USER2_CAN:
        case ELM329_PROTO_USER4_CAN:
        case ELM329_PROTO_USER5_CAN:
            return true;
    }
    return false;
}

bool elm329_is_can_29_bits_id(final ELM329Device* elm329) {
    switch(elm329->protocol) {
        case ELM329_PROTO_ISO_15765_4_CAN_2:
        case ELM329_PROTO_ISO_15765_4_CAN_4:
        case ELM329_PROTO_SAEJ1939:
        case ELM329_PROTO_SAEJ1939_2:
            return true;
    }
    return false;
}

bool elm329_iso15765_parse_response(final ELM329Device* elm329, final Vehicle* vehicle) {
    assert(elm329->eol != null);
    log_msg(LOG_WARNING, "Assuming auto formatting on");
    final int id_sz_chars;
    if ( elm329_is_can_11_bits_id(elm329) ) {
        id_sz_chars = 3;
    } else if ( elm329_is_can_29_bits_id(elm329) ) {
        id_sz_chars = 8;
    } else {
        log_msg(LOG_ERROR, "Not an obd can interface (wrong protocol)");
        return false;
    }
    final byte id_ascii[id_sz_chars+1];
    id_ascii[id_sz_chars] = 0;

    final ELMDevice* elm = (ELMDevice*)elm329;
    final Iso15765Conversation_list* conversations = Iso15765Conversation_list_new();
    SERIAL_Buffer *_ITERATE(elm,ELM_ISO15765_PARSE_RESPONSE_ITERATOR)
    Iso15765Conversation_list_free(conversations);
    return true;
}
