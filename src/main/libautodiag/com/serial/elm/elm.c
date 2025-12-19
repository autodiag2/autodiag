#include "libautodiag/com/serial/elm/elm.h"

#define ELM_STANDARD_OBD_MESSAGE_PARSE_RESPONSE_ITERATOR(ptr,end_ptr) \
    switch(elm->guess_response(ptr)) { \
        case DEVICE_RECV_DATA: { \
            final Buffer * bin_buffer = elm_ascii_to_bin_str((ELMDevice*)elm,ptr,end_ptr); \
            if ( bin_buffer == null ) { \
                return false; \
            } else { \
                obd_standard_parse_buffer(vehicle,bin_buffer); \
                buffer_free(bin_buffer); \
            } \
            break; \
        } \
    }

bool elm_standard_obd_message_parse_response(final ELMDevice* elm, final Vehicle* vehicle) {
    SERIAL_BUFFER_ITERATE(elm,ELM_STANDARD_OBD_MESSAGE_PARSE_RESPONSE_ITERATOR)
    return true;
}

int elm_guess_response(final char * buffer) {
    int res = serial_guess_response(buffer);
    if ( res == DEVICE_RECV_DATA ) {
        for(int i = 0; i < ELMResponseStrNumber; i++) {
            assert(ELMResponseStr[i] != null);
            if ( strncmp(buffer, ELMResponseStr[i], strlen(ELMResponseStr[i])) == 0 ) {
                return ELMResponseOffset + i;
            }
        }
    }
    return res;
}

int elm_linefeeds(final Serial * port, final bool state) {
    assert(port != null);

    char *original_eol = port->eol;
        
    if ( serial_send_at_command(port, "l%d", state) ) {
        port->eol =  strdup(state ? "\r\n" : "\r");
        buffer_recycle(port->recv_buffer);
        if ( port->recv(CAST_DEVICE(port)) == SERIAL_RESPONSE_OK ) {
            return state;
        } else {
            port->eol = original_eol;        
        }
    }
    return DEVICE_ERROR;
}

int elm_echo(final Serial * port, final bool state) {
    assert(port != null); 
    if ( serial_query_at_command(port,"e%d",state) ) {
        port->echo = state;
        return state;    
    } else {
        return DEVICE_ERROR;
    }
}

#define ELM_PRINT_ID_ITERATOR(ptr,end_ptr) \
    int recv_value = port->guess_response(ptr); \
    switch(recv_value) { \
        case DEVICE_RECV_DATA: { \
            char * eol = strstr(ptr,port->eol); \
            if ( eol != null ) { \
                *eol = 0; \
            }            \
            id = strdup(ptr); \
        } \
    }

char * elm_print_id(final Serial * port) {
    assert(port != null);
    
    serial_query_at_command(port,"i");
    
    char * id = null;
    
    buffer_ensure_termination(port->recv_buffer); 
    SERIAL_BUFFER_ITERATE(port,ELM_PRINT_ID_ITERATOR)
        
    return id;
}

ELMDevice* elm_open_from_serial_internal2(final Serial ** port) {
    ELMDevice * device = null;
    final char * response = elm_print_id(*port);
    final bool deviceConfigured;
    if ( response == null ) {
        deviceConfigured = false;
    } else {
        if (strstr(response, "ELM329") != null) {
            device = CAST_ELM_DEVICE(elm329_new_from_serial(*port));
        } else if (strstr(response, "ELM327") != null) {
            device = CAST_ELM_DEVICE((Device *)elm327_new_from_serial(*port));
        } else if (strstr(response, "ELM323") != null) {
            device = CAST_ELM_DEVICE((Device *)elm323_new_from_serial(*port));
        } else if (strstr(response, "ELM322") != null) {
            device = CAST_ELM_DEVICE((Device *)elm322_new_from_serial(*port));
        } else if (strstr(response, "ELM320") != null) {
            device = CAST_ELM_DEVICE((Device *)elm320_new_from_serial(*port));
        }
        if ( device == null ) {
            deviceConfigured = false;
        } else {
            deviceConfigured = device->configure((Device*)device);
        }
        free(response);
    }
    log_msg(LOG_DEBUG, "Info fetch done");

    if ( device != null ) {
        for(int i = 0;i < list_serial.size; i++) {
            if ( list_serial.list[i] == *port ) {
                log_msg(LOG_DEBUG, "Serial port changed to the real device");
                list_serial.list[i] = (Serial *)device;
                *port = list_serial.list[i];
                break;
            }
        }
    }
    if ( deviceConfigured ) {
        return device;
    } else {
        return null;
    }
}
ELMDevice* elm_open_from_serial_internal(final Serial * * port) {
    if ( elm_echo(*port,false) == DEVICE_ERROR ) {
        log_msg(LOG_ERROR, "Error while turn echo off");
        return null;
    }
    if ( elm_linefeeds(*port,false) == DEVICE_ERROR ) {
        log_msg(LOG_ERROR, "Error while turn line feeds off");
        return null;
    }

    return elm_open_from_serial_internal2(port);
}

void elm_debug(final ELMDevice * elm) {
    printf("ElmDevice: {\n");
    serial_debug((Serial*)elm);
    printf("    printing_of_spaces: %d\n", elm->printing_of_spaces);
    printf("    configure: %p\n", elm->configure);
    printf("}\n");
}

ELMDevice* elm_open_from_serial(final Serial * port) {
    assert(port != null);
    ELMDevice* device = elm_open_from_serial_internal(&port);
    if ( device == null ) {
        log_msg(LOG_DEBUG, "Configuration has failed, resetting and trying one more time");
        serial_query_at_command(port, "d");
        serial_reset_to_default(port);
        device = elm_open_from_serial_internal(&port);
        if ( device == null ) {
            log_msg(LOG_DEBUG, "Configuration has failed, device maybe not using AT&T default, trying one more time");
            elm_echo(port,false);
            port->echo = false;
            elm_linefeeds(port,false);
            port->eol = strdup("\r");
            device = elm_open_from_serial_internal2(&port);
        }
    }
    if ( device == null ) {
        log_msg(LOG_WARNING, "Everything has been tried but device config has failed");
    }
    return device;
}

void elm_ascii_to_bin_with_device(final ELMDevice * elm, final Buffer * bin, final char * ascii, final char * end_ptr) {
    elm_ascii_to_bin_internal(elm->printing_of_spaces,bin,ascii,end_ptr);
}

void elm_ascii_to_bin_internal(final bool printing_of_spaces, final Buffer * bin, final char * ascii, final char * end_ptr) {
    char hex[3];
    hex[2] = 0;
    for(char *ptr = ascii; (ptr+1) < end_ptr; ptr+=(2 + printing_of_spaces)) {
        buffer_ensure_capacity(bin,1);
        memcpy(hex, ptr, 2);
        bin->buffer[bin->size++] = (char)strtol(hex,null,16);
    }
}

char* elm_ascii_from_bin(final bool printing_of_spaces, final Buffer * bin) {
    char hex[4];
    hex[3] = 0;
    char * ascii = (char*)malloc((bin->size * (2 + printing_of_spaces) + 1) * sizeof(char));
    for(int i = 0; i < bin->size; i ++) {
        sprintf(ascii + i * (2 + printing_of_spaces),"%02X%s", bin->buffer[i], printing_of_spaces ? " " : "");
    }
    return ascii;
}

Buffer * elm_ascii_to_bin(final ELMDevice * elm, final Buffer * ascii) {
    final Buffer * bin = buffer_new();
    buffer_ensure_capacity(bin,ascii->size);
    elm_ascii_to_bin_with_device(elm, bin, (char*)ascii->buffer, (char*)ascii->buffer + ascii->size);
    return bin;
}

Buffer * elm_ascii_to_bin_str(final ELMDevice * elm, final char * ascii, final char * end_ptr) {
    final Buffer * bin = buffer_new();
    elm_ascii_to_bin_with_device(elm, bin, ascii, end_ptr);
    return bin;
}
bool elm_ensure_protocol_config_success(final ELMDevice* elm, final int protocol_max_value) {
    log_msg(LOG_DEBUG, "Detecting the connection sanity with show supported PIDS in current data");
    char * testerOrder = buffer_to_hex_string(buffer_from_ints(OBD_SERVICE_SHOW_CURRENT_DATA, 0x00));
    bool sanityCheck = false;
    int protocol = 1;
    // ensure that the SEARCHING... on bus init
    // is effectively awaited, we take the worst case: ISO9141 / KWP2000 init: ~250 ms – 1 s per attempt
    // for devices that got protocol choice
    // Total = sum of all protocol attempts → often 5–8 seconds worst case.
    final int oldTimeout = elm->timeout;
    final int second = 1000;
    elm->timeout = 10 * second;
    do {
        elm->send(CAST_DEVICE(elm), testerOrder);
        elm->clear_data(CAST_DEVICE(elm));
        final int detectionResult = elm->recv(CAST_DEVICE(elm));
        if ( detectionResult == DEVICE_RECV_DATA ) {
            sanityCheck = true;
        } else {
            elm->send(CAST_DEVICE(elm), at_command("tp %x", protocol));
            elm->clear_data(CAST_DEVICE(elm));
            if ( elm->recv(CAST_DEVICE(elm)) != SERIAL_RESPONSE_OK ) {
                log_msg(LOG_WARNING, "Cannot set the protocol correctly");
            }
            elm->protocol = protocol;
            protocol ++;
        }
    } while ( ! sanityCheck && protocol <= protocol_max_value);
    elm->timeout = oldTimeout;
    return sanityCheck;
}