#include "libautodiag/com/serial/elm/elm320.h"

int elm320_guess_response(final char * buffer) {
    int elm_guess = elm_guess_response(buffer);
    if ( elm_guess == DEVICE_RECV_DATA ) {
        for(int i = 0; i < ELM320ResponseStrNumber; i++) {
            assert(ELM320ResponseStr[i] != null);
            if ( strncmp(buffer, ELM320ResponseStr[i], strlen(ELM320ResponseStr[i])) == 0 ) {
                if ( i == ELM320_RESPONSE_NO_DATA ) {
                    return DEVICE_RECV_DATA_UNAVAILABLE;
                } else {
                    return ELM320_RESPONSE_OFFSET + i;
                }
            }
        }
        return DEVICE_RECV_DATA;
    } else {
        return elm_guess;
    }
}

char * elm320_describe_communication_layer(ELM320Device* elm320) {
    return strdup("SAE J1850 PWM (41.6 kBit/s)");
}

bool elm320_configure(final ELM320Device* elm320) {
    serial_query_at_command((Serial*)elm320,"fd"); 
    serial_query_at_command((Serial*)elm320,"h%d",true); 
    return true;
}

#define ELM320_RECV_ITERATOR(ptr,end_ptr) \
    int recv_value = elm320_guess_response(ptr); \
    switch(recv_value) { \
        case SERIAL_RESPONSE_PROMPT: \
            continue_reception = false; \
            break; \
        default: \
            deduced_response = recv_value; \
            break; \
    }
GEN_SERIAL_RECV(elm320_recv,ELM320Device,ELM320_RECV_ITERATOR)

void elm320_init(ELM320Device* d) {
    d->send = CAST_DEVICE_SEND(serial_send);
    d->recv = CAST_DEVICE_RECV(elm320_recv);
    d->describe_communication_layer = CAST_DEVICE_DESCRIBE_COMMUNICATION_LAYER(elm320_describe_communication_layer);
    d->parse_data = CAST_DEVICE_PARSE_DATA(elm_standard_obd_message_parse_response);
    d->guess_response = elm320_guess_response;
    d->configure = CAST_ELM_DEVICE_CONFIGURE(elm320_configure);
    d->printing_of_spaces = true;
}

ELM320Device* elm320_new() {
    ELM320Device* d = (ELM320Device*)malloc(sizeof(ELM320Device));
    elm320_init(d);
    return d;
}

ELM320Device* elm320_new_from_serial(final Serial *serial) {
    ELM320Device* d = elm320_new();
    memcpy(d,serial,sizeof(Serial));
    elm320_init(d);
    return d;
}
