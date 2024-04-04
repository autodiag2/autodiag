#include "com/serial/elm/elm323/elm323.h"

int elm323_guess_response(final char * buffer) {
    int elm_guess = elm_guess_response(buffer);
    if ( elm_guess == DEVICE_RECV_DATA ) {
        for(int i = 0; i < ELM323ResponseStrNumber; i++) {
            assert(ELM323ResponseStr[i] != null);
            if ( strncmp(buffer, ELM323ResponseStr[i], strlen(ELM323ResponseStr[i])) == 0 ) {
                if ( i == ELM323_RESPONSE_NO_DATA ) {
                    return DEVICE_RECV_DATA_UNAVAILABLE;
                } else {
                    return ELM323_RESPONSE_OFFSET + i;
                }
            }
        }
        return DEVICE_RECV_DATA;
    } else {
        return elm_guess;
    }
}

char * elm323_describe_communication_layer(ELM323Device* elm323) {
    return strdup("ISO 9141-2 / ISO 14230-4 (KWP2000)");
}

bool elm323_configure(final ELM323Device* elm323) {
    serial_query_at_command((Serial*)elm323,"fd"); 
    serial_query_at_command((Serial*)elm323,"h%d",true); 
    return true;
}

#define ELM323_RECV_ITERATOR(ptr,end_ptr) \
    int recv_value = elm323_guess_response(ptr); \
    switch(recv_value) { \
        case SERIAL_RESPONSE_PROMPT: \
            continue_reception = false; \
            break; \
        default: \
            deduced_response = recv_value; \
            break; \
    }
GEN_SERIAL_RECV(elm323_recv,ELM323Device,ELM323_RECV_ITERATOR)

void elm323_init(ELM323Device* d) {
    d->send = (int (*)(_Device *, const char *))serial_send;
    d->recv = (int (*)(_Device *))elm323_recv;
    d->describe_communication_layer = (char*(*)(_Device*))elm323_describe_communication_layer;
    d->parse_data = (bool (*)(_Device* , Vehicle*))elm_standard_obd_message_parse_response;    
    d->guess_response = elm323_guess_response;
    d->configure = (bool (*)(Device*))elm323_configure;
    d->printing_of_spaces = true;
}

ELM323Device* elm323_new() {
    ELM323Device* d = (ELM323Device*)malloc(sizeof(ELM323Device));
    elm323_init(d);
    return d;
}

ELM323Device* elm323_new_from_serial(final Serial *serial) {
    ELM323Device* d = elm323_new();
    memcpy(d,serial,sizeof(Serial));
    elm323_init(d);
    return d;
}
