#include "libautodiag/com/serial/elm/elm329/elm329.h"

bool elm329_is_can(final ELM329Device* elm329) {
    switch(elm329->protocol) {
        case ELM329_PROTO_ISO_15765_4_CAN_1:
        case ELM329_PROTO_ISO_15765_4_CAN_2:
        case ELM329_PROTO_ISO_15765_4_CAN_3:
        case ELM329_PROTO_ISO_15765_4_CAN_4:
        case ELM329_PROTO_USER1_CAN:
        case ELM329_PROTO_USER2_CAN:
        case ELM329_PROTO_SAEJ1939:
        case ELM329_PROTO_SAEJ1939_2:
        case ELM329_PROTO_USER4_CAN:
        case ELM329_PROTO_USER5_CAN:
            return true;
    }
    return false;
}

int elm329_guess_response(final char * buffer) {
    int elm_guess = elm_guess_response(buffer);
    if ( elm_guess == DEVICE_RECV_DATA ) {
        for(int i = 0; i < ELM329ResponseStrNumber; i++) {
            assert(ELM329ResponseStr[i] != null);
            if ( strncmp(buffer, ELM329ResponseStr[i], strlen(ELM329ResponseStr[i])) == 0 ) {
                if ( i == ELM329_RESPONSE_NO_DATA ) {
                    return DEVICE_RECV_DATA_UNAVAILABLE;
                } else {
                    return ELM329_RESPONSE_OFFSET + i;
                }
            }
        }
        return DEVICE_RECV_DATA;
    } else {
        return elm_guess;
    }
}

#define ELM329_CURRENT_PROTOCOL_ITERATOR(ptr,end_ptr) \
    switch(elm329_guess_response(ptr)) { \
        case DEVICE_RECV_DATA: { \
            final int chrs = strlen(ptr); \
            final bool isAuto = (1 < chrs && *ptr == 'A' ); \
            final byte * protocolLetter; \
            if ( isAuto ) { \
                protocolLetter = ptr + 1; \
            } else { \
                protocolLetter = ptr; \
            } \
            current_protocol = strtol(protocolLetter,null,16); \
            break; \
        } \
    }

ELM329_PROTO elm329_get_current_protocol(final ELM329Device* elm329) {
    final char * command = at_command("dpn");
    final bool result = (3 <= elm329->send(CAST_DEVICE(elm329), command));
    buffer_recycle(elm329->recv_buffer);
    elm329->recv(CAST_DEVICE(elm329));

    ELM329_PROTO current_protocol = ELM329_PROTO_NONE;
    SERIAL_BUFFER_ITERATE(elm329,ELM329_CURRENT_PROTOCOL_ITERATOR)

    free(command);
    return current_protocol;
}

bool elm329_obd_data_parse(final ELM329Device* elm329, final Vehicle* vehicle) {
    if ( elm329->protocol == ELM329_PROTO_NONE ) {
        return false;
    } else if ( elm329_is_can(elm329) ) {
        return elm329_iso15765_parse_response(elm329, vehicle);
    } else {
        return elm_standard_obd_message_parse_response((ELMDevice*)elm329, vehicle);
    }
}

int elm329_send(final ELM329Device* elm329, const char *command) {
    if ( 1 < strlen(command) ) {
        if ( ! at_is_command((char*)command) ) {
            char hex[3] = {0};
            memcpy(hex,command,2);
            int service = strtol(hex,null,16);
            switch(service) {
                case 0x01:
                case 0x02: {
                    char *commandLine;
                    asprintf(&commandLine,"%s%d",command,1);
                    int bytes = serial_send((SERIAL)elm329,commandLine);
                    free(commandLine);
                    return bytes;
                }
            }
        }
    }
    return serial_send((SERIAL)elm329,command);
}

#define ELM329_RECV_ITERATOR(ptr,end_ptr) \
    int recv_value = elm329_guess_response(ptr); \
    switch(recv_value) { \
        case SERIAL_RESPONSE_PROMPT: \
            continue_reception = false; \
            break; \
        case ELM329_RESPONSE_SEARCHING: \
            break; \
        default: \
            deduced_response = recv_value; \
            break; \
    }
GEN_SERIAL_RECV(elm329_recv,ELM329Device,ELM329_RECV_ITERATOR)

char* elm329_describe_communication_layer(final ELM329Device* elm329) {
    switch (elm329->protocol) {
        case ELM329_PROTO_NONE:                    return strdup("N/A");
        case ELM329_PROTO_SAE_J1850_1:             return strdup("SAE J1850 PWM (41.6 kBit/s)");
        case ELM329_PROTO_SAE_J1850_2:             return strdup("SAE J1850 VPW (10.4 kBit/s)");
        case ELM329_PROTO_ISO_9141_2:              return strdup("ISO 9141-2");
        case ELM329_PROTO_ISO_14230_4_KWP2000_1:   return strdup("ISO 14230-4 KWP2000 (5-baud init)");
        case ELM329_PROTO_ISO_14230_4_KWP2000_2:   return strdup("ISO 14230-4 KWP2000 (fast init)");
        case ELM329_PROTO_ISO_15765_4_CAN_1:       return strdup("ISO 15765-4 (CAN 11-bit ID, 500 kBit/s)");
        case ELM329_PROTO_ISO_15765_4_CAN_2:       return strdup("ISO 15765-4 (CAN 29-bit ID, 500 kBit/s)");
        case ELM329_PROTO_ISO_15765_4_CAN_3:       return strdup("ISO 15765-4 (CAN 11-bit ID, 250 kBit/s)");
        case ELM329_PROTO_ISO_15765_4_CAN_4:       return strdup("ISO 15765-4 (CAN 29-bit ID, 250 kBit/s)");
        case ELM329_PROTO_SAEJ1939:                return strdup("SAE J1939 (CAN 29-bit ID, 250 kBit/s)");
        case ELM329_PROTO_USER1_CAN:               return strdup("USER1 CAN");
        case ELM329_PROTO_USER2_CAN:               return strdup("USER2 CAN");
        case ELM329_PROTO_SAEJ1939_2:              return strdup("SAE J1939* CAN (29* bit ID, 500* kbaud)");
        case ELM329_PROTO_USER4_CAN:               return strdup("USER4 CAN (11* bit ID, 95.2* kbaud)");
        case ELM329_PROTO_USER5_CAN:               return strdup("USER5 CAN (11* bit ID, 33.3* kbaud)");         
    }
    return serial_describe_communication_layer((SERIAL)elm329);
}

bool elm329_configure(final ELM329Device* elm329) {
    elm329->protocol = elm329_get_current_protocol(elm329);
    if ( elm329->protocol != ELM329_PROTO_NONE ) {
        if ( ! serial_query_at_command((Serial*)elm329,"s%d",false) ) {
            log_msg(LOG_ERROR, "Error during printing of spaces");
        }
        elm329->printing_of_spaces = false;
        if ( elm329_is_can(elm329) ) {
            serial_query_at_command((Serial*)elm329,"caf%d",true);
        }
        serial_query_at_command((Serial*)elm329,"h%d",true);
        return true;
    }
    return false;
}

void elm329_init(ELM329Device* d) {
    d->send = CAST_DEVICE_SEND(elm329_send);
    d->recv = CAST_DEVICE_RECV(elm329_recv);
    d->describe_communication_layer = CAST_DEVICE_DESCRIBE_COMMUNICATION_LAYER(elm329_describe_communication_layer);
    d->parse_data = CAST_DEVICE_PARSE_DATA(elm329_obd_data_parse);
    d->protocol = ELM329_PROTO_NONE;
    d->guess_response = CAST_SERIAL_GUESS_RESPONSE(elm329_guess_response);
    d->configure = CAST_ELM_DEVICE_CONFIGURE(elm329_configure);
    d->printing_of_spaces = true;
}

ELM329Device* elm329_new() {
    ELM329Device* d = (ELM329Device*)malloc(sizeof(ELM329Device));
    elm329_init(d);
    return d;
}

ELM329Device* elm329_new_from_serial(final Serial *serial) {
    ELM329Device* d = elm329_new();
    memcpy(d,serial,sizeof(Serial));
    elm329_init(d);
    return d;
}
