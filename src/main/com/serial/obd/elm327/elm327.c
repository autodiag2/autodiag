#include "com/serial/obd/elm327/elm327.h"

int elm327_optimized_send(final OBDIFACE iface, const char *command, final int expected_lines_of_data) {
    char *commandLine;
    asprintf(&commandLine,"%s%d",command,expected_lines_of_data);
    final int bytes_sent = obd_send(iface,commandLine);
    free(commandLine);
    return bytes_sent;
}

bool elm327_standard_obd_message_parse_response(final OBDIFACE iface) {
    char *ptr = iface->recv_buffer->buffer, *end_ptr = strstr(iface->recv_buffer->buffer, iface->eol);
    if ( end_ptr == null ) {
        end_ptr = iface->recv_buffer->buffer + iface->recv_buffer->size_used - 1;
    }
    while(end_ptr != null) {
        final BUFFER bin_buffer = obd_serial_ascii_to_bin_str(iface,ptr,end_ptr);
        if ( iface->printing_of_headers ) {
            buffer_left_shift(bin_buffer,ELM327_OBD_MESSAGE_HEADER_STANDARD_SIZE);
        }
        if ( bin_buffer == null ) {
            return false;
        } else {
            buffer_list_append(iface->obd_data_buffer,bin_buffer);
        }
        ptr = end_ptr + 1;
        end_ptr = strstr(ptr, iface->eol);
    }
    return true;
}

bool elm327_parse_data_response(final OBDIFACE iface) {
    if ( iface->protocol == OBD_PROTO_NONE ) {
        return false;
    } else if ( obd_iface_is_can(iface) ) {
        return elm327_iso15765_parse_response(iface);
    } else {
        return elm327_standard_obd_message_parse_response(iface);
    }
}
int elm327_recv_blocking(final OBDIFACE iface) {
    if ( iface == null || iface->recv_buffer == null ) {
        return SERIAL_INTERNAL_ERROR;
    } else {
        int deduced_response = ELM327_RESPONSE_NULL;
        bool continue_reception = true;
        while(continue_reception) {
            final int buffer_offset = iface->recv_buffer->size_used;
            if ( 0 < iface->recv_buffer->size_used && iface->recv_buffer->buffer[iface->recv_buffer->size_used-1] == 0 ) {
                if ( strlen(iface->eol) < iface->recv_buffer->size_used && 
                    strcmp(iface->recv_buffer->buffer+iface->recv_buffer->size_used-1-strlen(iface->eol),iface->eol) == 0 
                ) {
                    log_msg("eol terminated line, no matter", LOG_DEBUG);
                } else {
                    iface->recv_buffer->size_used--;
                }
            }
            final int bytes_received = serial_recv((SERIAL)iface);
            if ( bytes_received == SERIAL_INTERNAL_ERROR ) {
                return SERIAL_INTERNAL_ERROR;
            } else {
                final byte *buffer = iface->recv_buffer->buffer + buffer_offset;
                if ( bytes_received == 0 || (bytes_received == 1 && (*buffer) == 0 ) ) {
                    return ELM327_RESPONSE_EMPTY;
                } else {
                    char *ptr = buffer, *end_ptr = strstr(buffer, iface->eol);
                    final char *ptr_end_excluded = buffer + bytes_received;
                    if ( end_ptr == null ) {
                        end_ptr = ptr_end_excluded - 1;
                    }
                    while(ptr != ptr_end_excluded) {
                        if ( ptr == end_ptr ) {
                            log_msg("Skip one empty line", LOG_DEBUG);
                        } else {
                            final int recv_value = elm327_guess_response(ptr);
                            switch(recv_value) {
                                case ELM327_RESPONSE_PROMPT:
                                    continue_reception = false;
                                    *ptr = 0; 
                                    ptr = ptr_end_excluded;
                                    continue;
                                case ELM327_RESPONSE_SEARCHING:
                                    break;
                                default:
                                    deduced_response = recv_value;
                                    if ( recv_value == ELM327_RESPONSE_NULL ) {
                                        deduced_response = ELM327_RESPONSE_DATA;
                                    }
                                    break;
                            }
                        }
                        ptr = end_ptr + 1;
                        end_ptr = strstr(ptr, iface->eol);
                        if ( end_ptr == null ) {
                            end_ptr = ptr_end_excluded - 1;
                        }
                    }
                }
            }
        }
        if ( deduced_response == ELM327_RESPONSE_DATA ) {
            elm327_parse_data_response(iface);
        }
        return deduced_response;
    }
}

ELM327Response elm327_guess_response(final char * buffer) {
    for(int i = 0; i < ELM327ResponseStrNumber; i++) {
        if ( ELM327ResponseStr[i] != null && 
            strstr(buffer, ELM327ResponseStr[i]) == (char*)buffer) {
            return i;
        }
    }
    return ELM327_RESPONSE_NULL;
}

int elm327_recv(final OBDIFACE iface) {
    if ( iface == null || iface->recv_buffer == null ) {
        return SERIAL_INTERNAL_ERROR;
    } else {
        final int buffer_offset = iface->recv_buffer->size_used;
        final int bytes_received = serial_recv((SERIAL)iface);
        if ( bytes_received == SERIAL_INTERNAL_ERROR ) {
            return SERIAL_INTERNAL_ERROR;
        } else {
            final byte *buffer = iface->recv_buffer->buffer + buffer_offset;
            if ( bytes_received == 0 || (bytes_received == 1 && (*buffer) == 0 ) ) {
                return ELM327_RESPONSE_EMPTY;
            } else {
                final ELM327Response response = elm327_guess_response(buffer);
                if ( response != ELM327_RESPONSE_NULL) {
                    return response;
                }
                elm327_parse_data_response(iface);
                return ELM327_RESPONSE_DATA;
            }
        }
    }
}

void elm327_send_wait_remote_confirmation(final nonnull OBDIFACE port) {
    do {
        buffer_recycle(port->recv_buffer);
    } while (elm327_recv(port)!=ELM327_RESPONSE_PROMPT);
    buffer_recycle(port->recv_buffer);
}

void elm327_configure(final OBDIFACE iface) {
    if ( obd_iface_is_can(iface) ) {
        serial_at_elm327_set_auto_formatting(iface,true);
        serial_at_elm327_set_headers(iface,true);
    } else {
        serial_at_elm327_set_headers(iface,false);
    }
}

