#include "libautodiag/com/serial/elm/elm322.h"

int elm322_guess_response(final char * buffer) {
    int elm_guess = elm_guess_response(buffer);
    if ( elm_guess == DEVICE_RECV_DATA ) {
        for(int i = 0; i < ELM322ResponseStrNumber; i++) {
            assert(ELM322ResponseStr[i] != null);
            if ( strncmp(buffer, ELM322ResponseStr[i], strlen(ELM322ResponseStr[i])) == 0 ) {
                if ( i == ELM322_RESPONSE_NO_DATA ) {
                    return DEVICE_RECV_DATA_UNAVAILABLE;
                } else {
                    return ELM322_RESPONSE_OFFSET + i;
                }
            }
        }
    }
    return elm_guess;
}

char * elm322_describe_communication_layer(ELM322Device* elm322) {
    return strdup("SAE J1850 VPW (10.4 kBit/s)");
}

bool elm322_configure(final ELM322Device* elm322) {
    ad_serial_query_at_command((Serial*)elm322,"fd"); 
    ad_serial_query_at_command((Serial*)elm322,"h%d",true); 
    return true;
}

#define ELM322_RECV_ITERATOR(ptr,end_ptr) \
    int recv_value = elm322_guess_response(ptr); \
    switch(recv_value) { \
        case SERIAL_RESPONSE_PROMPT: \
            continue_reception = false; \
            break; \
        default: \
            deduced_response = recv_value; \
            break; \
    }
GEN_SERIAL_RECV(elm322_recv,ELM322Device,ELM322_RECV_ITERATOR)

static bool proto_is_can(ELM322Device* d) {
    return false;
}
static bool fetch_current_protocol(ELM322Device * d) {
    return true;
}
void elm322_init(ELM322Device* d) {
    d->send = AD_DEVICE_SEND(ad_serial_send);
    d->recv = AD_DEVICE_RECV(elm322_recv);
    d->describe_communication_layer = AD_DEVICE_DESCRIBE_COMMUNICATION_LAYER(elm322_describe_communication_layer);
    d->parse_data = AD_DEVICE_PARSE_DATA(elm_standard_obd_message_parse_response);
    d->guess_response = AD_SERIAL_GUESS_RESPONSE(elm322_guess_response);
    d->configure = AD_ELM_DEVICE_CONFIGURE(elm322_configure);
    d->printing_of_spaces = true;
    d->proto_is_can = AD_ELM_DEVICE_PROTO_IS_CAN(proto_is_can);
    d->protocol = AD_DEVICE_ELM_PROTO_AUTO + 1;
    d->fetch_protocol = AD_DEVICE_ELM_FETCH_PROTOCOL(fetch_current_protocol);
}

ELM322Device* elm322_new() {
    ELM322Device* d = (ELM322Device*)malloc(sizeof(ELM322Device));
    elm322_init(d);
    return d;
}

ELM322Device* elm322_new_from_serial(final Serial *serial) {
    ELM322Device* d = elm322_new();
    memcpy(d,serial,sizeof(Serial));
    elm322_init(d);
    return d;
}
