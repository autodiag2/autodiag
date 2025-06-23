#include "libautodiag/com/serial/elm/elm327/elm327.h"

bool elm327_obd_data_parse(final ELM327Device* elm327, final Vehicle* vehicle) {
    if ( elm327->protocol == ELM327_PROTO_NONE ) {
        return false;
    } else if ( elm327_is_can(elm327) ) {
        return elm327_iso15765_parse_response(elm327, vehicle);
    } else {
        return elm_standard_obd_message_parse_response((ELMDevice*)elm327, vehicle);
    }
}

int elm327_guess_response(final char * buffer) {
    int elm_guess = elm_guess_response(buffer);
    if ( elm_guess == DEVICE_RECV_DATA ) {
        for(int i = 0; i < ELM327ResponseStrNumber; i++) {
            assert(ELM327ResponseStr[i] != null);
            if ( strncmp(buffer, ELM327ResponseStr[i], strlen(ELM327ResponseStr[i])) == 0 ) {
                if ( i == ELM327_RESPONSE_NO_DATA ) {
                    return DEVICE_RECV_DATA_UNAVAILABLE;
                } else {
                    return ELM327_RESPONSE_OFFSET + i;
                }
            }
        }
        return DEVICE_RECV_DATA;
    } else {
        return elm_guess;
    }
}

int elm327_send(final ELM327Device* elm327, const char *command) {
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
                    int bytes = serial_send((Serial *)elm327,commandLine);
                    free(commandLine);
                    return bytes;
                }
            }
        }
    }
    return serial_send((Serial *)elm327,command);
}

#define ELM327_RECV_ITERATOR(ptr,end_ptr) \
    int recv_value = elm327_guess_response(ptr); \
    switch(recv_value) { \
        case SERIAL_RESPONSE_PROMPT: \
            continue_reception = false; \
            break; \
        case ELM327_RESPONSE_SEARCHING: \
            break; \
        default: \
            deduced_response = recv_value; \
            break; \
    }
GEN_SERIAL_RECV(elm327_recv,ELM327Device,ELM327_RECV_ITERATOR)
bool elm327_protocol_is_j1939(final ELM327_PROTO protocol) {
    return protocol == ELM327_PROTO_SAEJ1939 ||
            protocol == ELM327_PROTO_USER1_CAN ||
            protocol == ELM327_PROTO_USER2_CAN;
}
bool elm327_configure(final ELM327Device* elm327) {
    elm327->protocol = elm327_get_current_protocol(elm327);
    if ( elm327->protocol != ELM327_PROTO_NONE ) {
        if ( ! elm327_printing_of_spaces(elm327,false) ) {
            log_msg(LOG_ERROR, "Error during printing of spaces");
        }
        if ( elm327_is_can(elm327) ) {
            elm327_set_auto_formatting(elm327,true);
            serial_query_at_command((Serial*)elm327,"d1");
        }
        serial_query_at_command((Serial*)elm327,"h%d",true);
        return true;
    }
    return false;
}
char* elm327_protocol_to_string(final ELM327_PROTO proto) {
    switch(proto) {
        case ELM327_PROTO_NONE:                    return strdup("N/A");
        case ELM327_PROTO_SAE_J1850_1:             return strdup("SAE J1850 PWM (41.6 kBit/s)");
        case ELM327_PROTO_SAE_J1850_2:             return strdup("SAE J1850 VPW (10.4 kBit/s)");
        case ELM327_PROTO_ISO_9141_2:              return strdup("ISO 9141-2");
        case ELM327_PROTO_ISO_14230_4_KWP2000_1:   return strdup("ISO 14230-4 KWP2000 (5-baud init)");
        case ELM327_PROTO_ISO_14230_4_KWP2000_2:   return strdup("ISO 14230-4 KWP2000 (fast init)");
        case ELM327_PROTO_ISO_15765_4_CAN_1:       return strdup("ISO 15765-4 (CAN 11-bit ID, 500 kBit/s)");
        case ELM327_PROTO_ISO_15765_4_CAN_2:       return strdup("ISO 15765-4 (CAN 29-bit ID, 500 kBit/s)");
        case ELM327_PROTO_ISO_15765_4_CAN_3:       return strdup("ISO 15765-4 (CAN 11-bit ID, 250 kBit/s)");
        case ELM327_PROTO_ISO_15765_4_CAN_4:       return strdup("ISO 15765-4 (CAN 29-bit ID, 250 kBit/s)");
        case ELM327_PROTO_SAEJ1939:                return strdup("SAE J1939 (CAN 29-bit ID, 250 kBit/s)");
        case ELM327_PROTO_USER1_CAN:               return strdup("USER1 CAN");
        case ELM327_PROTO_USER2_CAN:               return strdup("USER2 CAN");
    }
    return null;
}
char* elm327_describe_communication_layer(final ELM327Device* elm327) {
    char * res = elm327_protocol_to_string(elm327->protocol);
    if ( res == null ) {
        return serial_describe_communication_layer((Serial *)elm327);
    } else {
        return res;
    }
}

void elm327_init(ELM327Device* d) {
    d->send = CAST_DEVICE_SEND(elm327_send);
    d->recv = CAST_DEVICE_RECV(elm327_recv);
    d->describe_communication_layer = CAST_DEVICE_DESCRIBE_COMMUNICATION_LAYER(elm327_describe_communication_layer);
    d->parse_data = CAST_DEVICE_PARSE_DATA(elm327_obd_data_parse);
    d->guess_response = CAST_SERIAL_GUESS_RESPONSE(elm327_guess_response);
    d->configure = CAST_ELM_DEVICE_CONFIGURE(elm327_configure);
    d->protocol = ELM327_PROTO_NONE;
    d->printing_of_spaces = true;
}

ELM327Device* elm327_new() {
    ELM327Device* d = (ELM327Device*)malloc(sizeof(ELM327Device));
    elm327_init(d);
    return d;
}

ELM327Device* elm327_new_from_serial(final Serial *serial) {
    ELM327Device* d = elm327_new();
    memcpy(d,serial,sizeof(Serial));
    elm327_init(d);
    return d;
}

bool elm327_printing_of_spaces(final ELM327Device* elm327, bool state) {
    if ( serial_query_at_command((Serial*)elm327,"s%d",state) ) {
        elm327->printing_of_spaces = state;
        return true;
    } else {
        return false;
    }
}

#define ELM327_CURRENT_PROTOCOL_ITERATOR(ptr,end_ptr) \
    switch(elm327_guess_response(ptr)) { \
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

ELM327_PROTO elm327_get_current_protocol(final ELM327Device* elm327) {
    final char * command = at_command("dpn");
    final bool result = (3 <= elm327->send(CAST_DEVICE(elm327), command));
    buffer_recycle(elm327->recv_buffer);
    elm327->recv(CAST_DEVICE(elm327));

    ELM327_PROTO current_protocol = ELM327_PROTO_NONE;
    SERIAL_BUFFER_ITERATE(elm327,ELM327_CURRENT_PROTOCOL_ITERATOR)

    free(command);
    return current_protocol;
}

bool elm327_calibrate_battery_voltage(final ELM327Device* elm327, double voltage) {
    return serial_query_at_command((Serial*)elm327,"cv%d",(int)(voltage * 100));
}

#define ELM327_CURRENT_BATTERY_VOLTAGE_ITERATOR(ptr,end_ptr) \
    switch(elm327_guess_response(ptr)) { \
        case DEVICE_RECV_DATA: { \
            result = strtod(ptr,null); \
            break; \
        } \
    }

double elm327_get_current_battery_voltage(final ELM327Device* elm327) {
    final char * command = at_command("rv");
    elm327->send(CAST_DEVICE(elm327), command);
    buffer_recycle(elm327->recv_buffer);
    elm327->recv(CAST_DEVICE(elm327));

    final double result = -1;
    SERIAL_BUFFER_ITERATE(elm327,ELM327_CURRENT_BATTERY_VOLTAGE_ITERATOR)
    free(command);
    return result;
}

bool elm327_set_auto_formatting(final ELM327Device* elm327, final bool state) {
    return serial_query_at_command((Serial*)elm327,"caf%d",state);
}

bool elm327_protocol_is_can(final ELM327_PROTO proto) {
    switch(proto) {
        case ELM327_PROTO_ISO_15765_4_CAN_1:
        case ELM327_PROTO_ISO_15765_4_CAN_2:
        case ELM327_PROTO_ISO_15765_4_CAN_3:
        case ELM327_PROTO_ISO_15765_4_CAN_4:
        case ELM327_PROTO_USER1_CAN:
        case ELM327_PROTO_USER2_CAN:
        case ELM327_PROTO_SAEJ1939:
            return true;
    }
    return false;
}

bool elm327_is_can(final ELM327Device* elm327) {
    return elm327_protocol_is_can(elm327->protocol);
}
