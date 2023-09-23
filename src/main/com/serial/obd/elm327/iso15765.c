#include "com/serial/obd/elm327/iso15765.h"

bool elm327_can_is_11_bits_id(final OBDIFACE iface) {
    switch(iface->protocol) {
        case OBD_PROTO_ISO_15765_4_CAN_1:
        case OBD_PROTO_ISO_15765_4_CAN_3:
        case OBD_PROTO_USER1_CAN:
        case OBD_PROTO_USER2_CAN:
            return true;
    }
    return false;
}

bool elm327_can_is_29_bits_id(final OBDIFACE iface) {
    switch(iface->protocol) {
        case OBD_PROTO_ISO_15765_4_CAN_2:
        case OBD_PROTO_ISO_15765_4_CAN_4:
        case OBD_PROTO_SAEJ1939:
            return true;
    }
    return false;
}

int oneHex(char c) {
    char hex[2] = {0};
    hex[0] = c;
    return (int)strtol(hex,null,16);
}

bool elm327_iso15765_parse_response(final OBDIFACE iface) {
    log_msg("Assuming headers on and auto formatting on", LOG_WARNING);
    final int id_sz_chars;
    if ( elm327_can_is_11_bits_id(iface) ) {
        id_sz_chars = 3;
    } else if ( elm327_can_is_29_bits_id(iface) ) {
        id_sz_chars = 8;
    } else {
        log_msg("Error", LOG_ERROR);
        return false;
    }
    final byte id_ascii[id_sz_chars+1];
    id_ascii[id_sz_chars] = 0;

    char *ptr = iface->recv_buffer->buffer, *end_ptr = strstr(iface->recv_buffer->buffer, iface->eol);
    if ( end_ptr == null ) {
        end_ptr = iface->recv_buffer->buffer + iface->recv_buffer->size_used - 1;
    }
    bool consecutive_flow_state = false;
    BUFFER pending_buffer = null;
    while(end_ptr != null) {
        if ( strlen(ptr) == strlen(iface->eol) && strncmp(ptr,iface->eol,strlen(iface->eol)) == 0 ) {
            log_msg("Found one empty line", LOG_DEBUG);
        } else {
            log_msg("Parsing one CAN message", LOG_INFO);
            memcpy(id_ascii,ptr,id_sz_chars);
            ptr += id_sz_chars + iface->printing_of_spaces;
            final Iso15765FrameType frame_type = oneHex(*ptr);
            ptr ++;
            switch(frame_type) {    
                case Iso15765FirstFrame:
                    consecutive_flow_state = true;
                    break;
                case Iso15765ConsecutiveFrame:
                    break;
                default:
                    consecutive_flow_state = false;
            }
            switch(frame_type) {
                case Iso15765SingleFrame: {
                    log_msg("single frame", LOG_DEBUG);
                    final int data_bytes = oneHex(*ptr);
                    ptr += 1 + iface->printing_of_spaces;
                } break;
                case Iso15765FirstFrame: {
                    log_msg("first frame", LOG_DEBUG);
                    final int data_bytes = (oneHex(*(ptr+1+iface->printing_of_spaces)) << 8) + (oneHex(*(ptr+2+iface->printing_of_spaces)) << 4) + oneHex(*ptr);
                    ptr += 3 + iface->printing_of_spaces * 2;
                } break;
                case Iso15765ConsecutiveFrame: {
                    if ( consecutive_flow_state ) {
                        final int sequence_number = oneHex(*ptr);
                        ptr += 1 + iface->printing_of_spaces;
                    } else {
                        log_msg("consecutive frame without first frame : aborting", LOG_ERROR);
                        return false;
                    }
                } break;
                case Iso15765FlowControlFrame: {
                    log_msg("flow control frame : Generally unused frame", LOG_WARNING);
                } break;
                default: {
                    log_msg("Error", LOG_ERROR);
                    return false;
                }
            }
            final BUFFER bin_buffer = obd_serial_ascii_to_bin_str(iface,ptr,end_ptr);
            if ( bin_buffer == null ) {
                log_msg("Error", LOG_ERROR);
                return false;
            } else {
                switch(frame_type) {
                    case Iso15765FirstFrame:
                    case Iso15765SingleFrame:
                    case Iso15765FlowControlFrame:
                        if ( pending_buffer != null ) {
                            buffer_list_append(iface->obd_data_buffer,pending_buffer);
                            buffer_free(pending_buffer);
                            pending_buffer = null;
                        }
                }
                switch(frame_type) {
                    case Iso15765FirstFrame:
                        pending_buffer = buffer_new();
                        buffer_append(pending_buffer, bin_buffer);
                        break;
                    case Iso15765ConsecutiveFrame:
                        log_msg("Warning there is no management of misordered frames", LOG_DEBUG);
                        buffer_append(pending_buffer, bin_buffer);
                        break;
                    case Iso15765SingleFrame:
                        buffer_list_append(iface->obd_data_buffer,bin_buffer);
                        break;
                }
            }

        }
        ptr = end_ptr + 1;
        end_ptr = strstr(ptr, iface->eol);
    }
    if ( pending_buffer != null ) {
        buffer_list_append(iface->obd_data_buffer,pending_buffer);
        buffer_free(pending_buffer);
        pending_buffer = null;
    }
    return true;
}
