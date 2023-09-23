#include "com/serial/obd/obd.h"

void obd_lock(final nonnull OBDIFACE iface) {
    serial_lock((SERIAL)iface);
}

void obd_unlock(final nonnull OBDIFACE iface) {
    serial_unlock((SERIAL)iface);
}

int obd_serial_send_with_one_line_expected(final OBDIFACE iface, const char *command) {
    switch(iface->type) {
        case OBD_INTERFACE_ELM327: return elm327_optimized_send(iface, command, 1);
    }
    return obd_send(iface,command);
}

int obd_send(final OBDIFACE iface, const char *request) {
    return serial_send((SERIAL)iface, request);
}

void obd_clear_data(final OBDIFACE iface) {
    buffer_recycle(iface->recv_buffer);
    buffer_list_empty(iface->obd_data_buffer);
}

double obd_request_one_line_double(final OBDIFACE iface, void * (*responseHandler)(final OBDIFACE iface, final int response, void *arg), char *obd_request) {
    double *ptr = obd_request_one_line(iface, responseHandler, obd_request);
    double rv = *ptr;
    free(ptr);
    return rv;
}

int obd_request_one_line_int(final OBDIFACE iface, void * (*responseHandler)(final OBDIFACE iface, final int response, void *arg), char *obd_request) {
    int *ptr = obd_request_one_line(iface, responseHandler, obd_request);
    int rv = *ptr;
    free(ptr);
    return rv;
}

bool obd_request_one_line_bool(final OBDIFACE iface, void * (*responseHandler)(final OBDIFACE iface, final int response, void *arg), char *obd_request) {
    bool *ptr = obd_request_one_line(iface, responseHandler, obd_request);
    bool rv = *ptr;
    free(ptr);
    return rv;
}

void* obd_request_one_line(final OBDIFACE iface, void * (*responseHandler)(final OBDIFACE iface, final int response, void *arg), char *obd_request_str) {
    return obd_request(iface,responseHandler,null,obd_request_str,true);
}

void* obd_request(final OBDIFACE iface, void * (*responseHandler)(final OBDIFACE iface, final int response, void *arg), void * arg, char *obd_request_str, bool oneLineOptimized) {
    obd_lock(iface);
    int response = 0;
    void *ptr = null;
    for(int b = 0; b < iface->send_retry && response == 0; b++) {
        if ( oneLineOptimized ) {
            obd_serial_send_with_one_line_expected(iface,obd_request_str); 
        } else {
            obd_send(iface, obd_request_str);
        }
        obd_clear_data(iface);
        response = obd_recv_with_expected_databytes_start(iface,obd_request_str);
        ptr = responseHandler(iface,response,arg);
    }
    obd_unlock(iface);
    return ptr;
}

int obd_recv_remove_empty_frames(final OBDIFACE iface) {
    for(int i = 0; i < iface->obd_data_buffer->size; i++) {
        final BUFFER buffer = iface->obd_data_buffer->list[i];
        if ( buffer->size_used == 0 ) {
            buffer_list_remove(iface->obd_data_buffer, i);
            i--;
        }
    }
}

int obd_recv(final OBDIFACE iface, final bool (*filter)(final BUFFER buffer, final OBDIFACE iface, final void * arg),void *arg) {
    final int initial_data_buffer_received = iface->obd_data_buffer->size;
    switch ( iface->type) {
        case OBD_INTERFACE_ELM327: {
            switch(elm327_recv_blocking(iface)) {
                case ELM327_RESPONSE_NO_DATA:
                    return OBD_RECV_ERROR;
            }
        } break;
        default: {
            log_msg("Error adaptater not implemented", LOG_ERROR);
        }
    }
    for(int i = 0; i < iface->obd_data_buffer->size; i++) {
        final BUFFER data = iface->obd_data_buffer->list[i];
        if ( 0 < data->size_used ) {
            if ( filter == null || filter(data, iface, arg) ) {
                final byte service_id = buffer_extract_0(data);
                if ( service_id == 0x7F ) {
                    log_msg("KWP2000 error code encountered ", LOG_DEBUG);
                    continue;
                } else {
                    service_id &= ~0x40;
                    switch(service_id) {
                        case OBD_SERVICE_SHOW_CURRENT_DATA:
                        case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA:
                        case OBD_SERVICE_TESTS_RESULTS:
                        case OBD_SERVICE_TESTS_RESULTS_OTHER: {
                            if ( 0 < data->size_used ) {
                                final byte pid = buffer_extract_0(data);
                            } else {
                                log_msg("Expected a pid found nothing", LOG_DEBUG);
                            }
                        } break;
                        case OBD_SERVICE_CONTROL_OPERATION:
                        case OBD_SERVICE_PENDING_DTC:
                        case OBD_SERVICE_NONE:
                        case OBD_SERVICE_SHOW_DTC:
                        case OBD_SERVICE_CLEAR_DTC:
                        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION:
                        case OBD_SERVICE_PERMANENT_DTC:
                            break;
                    }
                }
            } else {
                buffer_list_remove(iface->obd_data_buffer,i);
                i--;
            }
        } else {
            log_msg("Expected a sid found nothing", LOG_DEBUG);
        }
    }
    return iface->obd_data_buffer->size - initial_data_buffer_received;
}

int obd_recv_with_expected_databytes_start(final OBDIFACE iface, final char * ascii_hex_bytes_maybe_const) {
    final BUFFER bytes = buffer_new();
    final char *ascii_hex_bytes = strdup(ascii_hex_bytes_maybe_const);
    final int bytes_n = strlen(ascii_hex_bytes)/2;
    buffer_ensure_capacity(bytes, bytes_n);
    for(int i = bytes_n; 0 < i; i--) {
        ascii_hex_bytes[i*2] = 0;
        bytes->buffer[i-1] = strtol(&(ascii_hex_bytes[(i-1)*2]),null,16);
    }
    bytes->size_used = bytes_n;
    int rv = obd_recv(iface,buffer_filter_expected_databytes_start,bytes);
    buffer_free(bytes);
    free(ascii_hex_bytes);
    return rv;
}

void user_feedback_alert(char *title, char *description, char * buttonA, char * buttonB) {
    printf("%s: %s (%s/%s)\n",(title==NULL?"":title),(description==NULL?"":description),(buttonA==NULL?"":buttonA),(buttonB==NULL?"":buttonB));
}

const char *obd_get_protocol_string(final OBD_INTERFACE_TYPE interface_type, final OBD_PROTOCOLE_TYPE protocol_id) {
   switch (interface_type) {
      case OBD_INTERFACE_ELM320: return OBD_PROTO_DESC_SAE_J1850_1;
      case OBD_INTERFACE_ELM322: return OBD_PROTO_DESC_SAE_J1850_2;
      case OBD_INTERFACE_ELM323: return OBD_PROTO_DESC_HACK_ELM323;
      case OBD_INTERFACE_ELM327:
      case OBD_INTERFACE_OBDLINK:
         switch (protocol_id) {
            case OBD_PROTO_NONE: return OBD_PROTO_DESC_NONE;
            case OBD_PROTO_SAE_J1850_1: return OBD_PROTO_DESC_SAE_J1850_1;
            case OBD_PROTO_SAE_J1850_2: return OBD_PROTO_DESC_SAE_J1850_2;
            case OBD_PROTO_ISO_9141_2: return OBD_PROTO_DESC_ISO_9141_2;
            case OBD_PROTO_ISO_14230_4_KWP2000_1: return OBD_PROTO_DESC_ISO_14230_4_KWP2000_1;
            case OBD_PROTO_ISO_14230_4_KWP2000_2: return OBD_PROTO_DESC_ISO_14230_4_KWP2000_2;
            case OBD_PROTO_ISO_15765_4_CAN_1: return OBD_PROTO_DESC_ISO_15765_4_CAN_1;
            case OBD_PROTO_ISO_15765_4_CAN_2: return OBD_PROTO_DESC_ISO_15765_4_CAN_2;
            case OBD_PROTO_ISO_15765_4_CAN_3: return OBD_PROTO_DESC_ISO_15765_4_CAN_3;
            case OBD_PROTO_ISO_15765_4_CAN_4: return OBD_PROTO_DESC_ISO_15765_4_CAN_4;
            case OBD_PROTO_SAEJ1939: return OBD_PROTO_DESC_SAEJ1939;
            case OBD_PROTO_USER1_CAN: return OBD_PROTO_DESC_USER1_CAN;
            case OBD_PROTO_USER2_CAN: return OBD_PROTO_DESC_USER2_CAN;
         }
   }   
   return OBD_PROTO_DESC_UNKNOWN;
}

void module_init_obd() {
    
}

OBDIFACE obd_new() {
    final OBDIFACE iface = (OBDIFACE)malloc(sizeof(OBDIFace));
    iface->type = OBD_INTERFACE_UNKNOWN;
    iface->protocol = OBD_PROTO_NONE;
    iface->printing_of_spaces = true;
    iface->printing_of_headers = false;
    return iface;
}

OBDIFACE obd_new_from_serial(final nonnull SERIAL serial) {
    assert(serial != null);
    final nonnull OBDIFACE iface = obd_new();
    memcpy(iface,serial,sizeof(Serial));
    iface->obd_data_buffer = buffer_list_new();
    return iface;
}
OBDIFACE obd_open_from_serial(final SERIAL port) {
    if ( port == null ) {
        return null;
    } else {
        if ( serial_open(port) == GENERIC_FUNCTION_ERROR ) {
            module_debug(MODULE_OBD "Error while openning OBD port");
        } else {
            serial_wait_until_prompt(port);
            final bool interfaceConfigured;
            final OBDIFACE iface = obd_new_from_serial(port);
            if ( serial_at_linefeeds((SERIAL)iface,false) == SERIAL_INTERNAL_ERROR ) {
                log_msg("Error while turn line feeds off", LOG_ERROR);
                return null;
            }
            if ( serial_at_echo((SERIAL)iface,false) == SERIAL_INTERNAL_ERROR ) {
                log_msg("Error while turn echo off", LOG_ERROR);
                return null;
            }
            if ( obd_interface_fill_type(iface) ) {
                if ( obd_interface_fill_protocol(iface) ) {
                    if ( ! serial_obd_interface_printing_of_spaces(iface, false) ) {
                        log_msg("Error during printing of spaces", LOG_ERROR);
                    }
                    switch (iface->type) {
                        case OBD_INTERFACE_ELM327: {
                            elm327_configure(iface);
                        }
                    }
                    interfaceConfigured = true;
                } else {
                    interfaceConfigured = false;
                }
            } else {
                interfaceConfigured = false;
            }
            module_debug(MODULE_OBD "Info fetch done");

            if ( interfaceConfigured ) {
                for(int i = 0;i < serial_list_size; i++) {
                    if ( serial_list[i] == port ) {
                        module_debug(MODULE_OBD "Serial port changed to OBD interface");
                        serial_list[i] = (SERIAL)iface;
                        free(port);
                        break;
                    }
                }
                return iface;
            } else {
                obd_free_only_obd_part(iface);
                return null;
            }
        }
    }
}

void obd_free_only_obd_part(final OBDIFACE iface) {
    buffer_list_free(iface->obd_data_buffer);
}
void obd_free(final OBDIFACE iface) {
    serial_close((SERIAL)iface);
    serial_free((SERIAL)iface);
    obd_free_only_obd_part(iface);
}
void obd_close(final OBDIFACE iface) {

    serial_close((SERIAL)iface);
}

void obd_serial_ascii_to_bin_internal(final OBDIFACE iface, final BUFFER bin, final char * ascii, final char * end_ptr) {
    char hex[3];
    hex[2] = 0;
    for(char *ptr = ascii; ptr != end_ptr && (ptr+1) != end_ptr && (ptr+1+iface->printing_of_spaces) != end_ptr; ptr+=(2 + iface->printing_of_spaces)) {
        buffer_ensure_capacity(bin,1);
        memcpy(hex, ptr, 2);
        bin->buffer[bin->size_used++] = (char)strtol(hex,null,16);
    }
}

BUFFER obd_serial_ascii_to_bin(final OBDIFACE iface, final BUFFER ascii) {
    final BUFFER bin = buffer_new();
    buffer_ensure_capacity(bin,ascii->size_used);
    obd_serial_ascii_to_bin_internal(iface, bin, ascii->buffer, ascii->buffer + ascii->size_used);
    return bin;
}

BUFFER obd_serial_ascii_to_bin_str(final OBDIFACE iface, final char * ascii, final char * end_ptr) {
    final BUFFER bin = buffer_new();
    obd_serial_ascii_to_bin_internal(iface, bin, ascii, end_ptr);
    return bin;
}

void obd_dump(final OBDIFACE iface) {
    serial_dump((SERIAL)iface);
    const char * title = "OBD dump (iface)";
    char *result = null;
    if ( iface == null ) {
        asprintf(&result, "%s: NULL", title);
    } else {
        asprintf(&result, "%s: %d", title, iface->type);
    }
    module_debug(MODULE_OBD result);
    free(result);
}

bool buffer_filter_expected_databytes_start(final BUFFER buffer, final OBDIFACE iface, void *arg) {
    final BUFFER bytes = (BUFFER)arg;
    if ( obd_iface_is_can(iface) && 0 < bytes->size_used ) {
        bytes->buffer[0] |= 0x40;
    }
    return memcmp(bytes->buffer,buffer->buffer,bytes->size_used < buffer->size_used ? bytes->size_used : buffer->size_used) == 0;
}
