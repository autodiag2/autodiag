#include "libautodiag/sim/elm327/bus.h"

/**
 * Response of the controller to the tester.
 */
static Buffer* data_extract_if_accepted(SimELM327* elm327, SimECU * ecu, Buffer * binRequest) {
    Buffer * dataRequest = buffer_new();
    log_msg(LOG_DEBUG, "Receving incoming request by the tester");
    log_msg(LOG_DEBUG, "TODO: addressing of ECUs in the bus, for now all ECUs receive all messages");
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            assert(4 <= binRequest->size);
            buffer_slice_append(dataRequest, binRequest, 4, binRequest->size - 4);
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            assert(2 <= binRequest->size);
            buffer_slice_append(dataRequest, binRequest, 2, binRequest->size - 2);
        } else {
            log_msg(LOG_WARNING, "Missing case here");
        }
        if ( elm327->can.extended_addressing ) {
            buffer_left_shift(dataRequest, 1);
        }
        final int pci_sz = 1;
        assert(0 < dataRequest->size);
        final int pci_sz_value = dataRequest->buffer[0] & 0x0F;
        buffer_left_shift(dataRequest, pci_sz);
        if ( pci_sz_value != dataRequest->size ) {
            log_msg(LOG_ERROR, "Generated pci is different than the actual request size (%d/%d)", pci_sz_value, dataRequest->size);
            //assert(pci_sz_value == dataRequest->size); TODO
        }
    } else {
        assert(3 <= binRequest->size);
        buffer_slice_append(dataRequest, binRequest, 3, binRequest->size - 3);
    }
    return dataRequest;
}

/**
 * Request by the tester to the vehicle (emu).
 * @param ecu to which send the dataRequest
 * @param dataRequest OBD/UDS
 * @return header to send to the vehicle (emu).
 */
static Buffer* request_header(SimELM327* elm327, SimECU * ecu, Buffer * dataRequest) {
    log_msg(LOG_DEBUG, "Generating the tester request header");
    Buffer *protocolSpecificHeader = buffer_new();
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            if ( elm327->custom_header->size == 3 ) {
                BUFFER_APPEND_BYTES(protocolSpecificHeader,
                    elm327->can.priority_29bits,
                    elm327->custom_header->buffer[0],
                    elm327->custom_header->buffer[1], 
                    elm327->custom_header->buffer[2]
                );
            } else if ( elm327->custom_header->size == 4 ) {
                buffer_append(protocolSpecificHeader, elm327->custom_header);
            } else {
                BUFFER_APPEND_BYTES(protocolSpecificHeader,
                    elm327->can.priority_29bits, 0xDA, 0xF1, elm327->testerAddress
                );
            }
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            if ( elm327->custom_header->size == 2 ) {
                BUFFER_APPEND_BYTES(protocolSpecificHeader,
                    elm327->custom_header->buffer[0]&0xF, 
                    elm327->custom_header->buffer[1]
                );
            } else if ( elm327->custom_header->size == 3 ) {
                BUFFER_APPEND_BYTES(protocolSpecificHeader,
                    elm327->custom_header->buffer[1]&0xF, 
                    elm327->custom_header->buffer[2]
                );
            } else {
                BUFFER_APPEND_BYTES(protocolSpecificHeader,
                    0x7, elm327->testerAddress
                );
            }
        } else {
            log_msg(LOG_WARNING, "Missing case here");
        }
        if ( elm327->can.extended_addressing ) {
            BUFFER_APPEND_BYTES(protocolSpecificHeader,
                elm327->can.extended_addressing_target_address
            );
        }
        if ( elm327->can.auto_format ) {
            final int pci = dataRequest->size;
            BUFFER_APPEND_BYTES(
                protocolSpecificHeader,
                pci   
            );
        }
    } else {
        if ( elm327->custom_header->size == 3 ) {
            buffer_append(protocolSpecificHeader, elm327->custom_header);
        } else {
            BUFFER_APPEND_BYTES(protocolSpecificHeader,
                0x41, 0x6B, elm327->testerAddress
            );
        }
    }
    return protocolSpecificHeader;     
}

char * sim_elm327_bus(SimELM327 * elm327, char * hex_string_request) {
    char *response = null;
    bool isHexString = true;
    bool hasSpaces = false;
    for(int i = 0; i < strlen(hex_string_request); i++) {
        char c = hex_string_request[i];
        if ( c == ' ' ) {
            hasSpaces = true;
        }
        if ( !( c == SIM_ELM327_PP_GET(elm327,0x0D) || c == SIM_ELM327_PP_GET(elm327,0x0A) || c == ' ' || 0x30 <= c && c <= 0x39 || 0x41 <= c && c <= 0x46 || 0x61 <= c && c <= 0x66 ) ) {
            isHexString = false;
        }
    }
    if ( isHexString && elm327->responses ) {
        
        final Buffer * dataRequest = buffer_new();
        char * end_ptr = strstr(hex_string_request,elm327->eol);
        elm_ascii_to_bin_internal(hasSpaces, dataRequest, hex_string_request, end_ptr == null ? hex_string_request + strlen(hex_string_request): end_ptr);
        log_msg(LOG_DEBUG, "TODO: handle multiple data request");
        
        for(int i = 0; i < LIST_SIM_ECU(elm327->ecus)->size; i++) {
            SimECU * ecu = LIST_SIM_ECU(elm327->ecus)->list[i];

            if ( ! elm327_protocol_is_j1939(elm327->protocolRunning) ) {
                if ( elm327->receive_address != null && *elm327->receive_address != ecu->address) {
                    continue;
                }
            }

            Buffer * binRequest = buffer_new();
            final Buffer * requestHeader = request_header(elm327, ecu, dataRequest);
            buffer_append(binRequest, requestHeader);
            buffer_append(binRequest, dataRequest);

            final Buffer * extractedDataRequest = data_extract_if_accepted((SimELM327*)elm327, ecu, binRequest);
            if ( extractedDataRequest->size == 0 ) {
                log_msg(LOG_DEBUG, "Not addressed to this ECU");
                continue;
            }
            char * tmpResponse = ecu->sim_ecu_response((SimELM327 *)elm327,ecu,extractedDataRequest);

            Buffer * response_header_bin = sim_ecu_generate_response_header_bin(elm327,ecu,ELM327_CAN_28_BITS_DEFAULT_PRIO);
            if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
                assert(elm327->can.mask != null);
                assert(elm327->can.filter != null);
                assert(elm327->can.mask->size == elm327->can.filter->size);
                if ( response_header_bin->size == elm327->can.mask->size ) {
                    bool filtered = false;
                    for(int i = 0; i < elm327->can.mask->size; i ++) {
                        byte m = elm327->can.mask->buffer[i];
                        int cmp1 = (response_header_bin->buffer[i] & m);
                        int cmp2 = (elm327->can.filter->buffer[i] & m);
                        if ( cmp1 != cmp2 ) {
                            filtered = true;
                            break;
                        }
                    }
                    if ( filtered ) {
                        free(tmpResponse);
                        tmpResponse = null;
                    }
                } else {
                    if ( 0 < elm327->can.mask->size ) {
                        log_msg(LOG_WARNING, "A mask is set but with the wrong format for protocol - filtering will not work");
                    }
                }
            }

            int sz = min(response_header_bin->size,12);
            buffer_recycle(elm327->obd_buffer);
            buffer_ensure_capacity(elm327->obd_buffer,sz);
            memmove(elm327->obd_buffer->buffer, response_header_bin->buffer, sz);
            elm327->obd_buffer->size = sz;

            if ( tmpResponse != null ) {
                char * tmpResponseResult;
                asprintf(&tmpResponseResult,"%s%s%s",response==null?"":response,tmpResponse,i+1<LIST_SIM_ECU(elm327->ecus)->size ? elm327->eol : "");
                tmpResponse = null; // we do not free to compat with python
                response = null;    // we do not free to compat with python
                response = tmpResponseResult;
            }
        }
        if ( response == null ) {
            double part = 1;
            if ( ! elm327_protocol_is_j1939(elm327->protocolRunning) && 
                    elm327->vehicle_response_timeout_adaptive ) {
                part = rand() / (1.0 * RAND_MAX);
            }
            useconds_t timeout_usec = elm327->vehicle_response_timeout * 1e3 * part;
            if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
                timeout_usec *= elm327->can.timeout_multiplier;
            }
            usleep(timeout_usec);
            asprintf(&response,"%s%s", ELM327ResponseStr[ELM327_RESPONSE_NO_DATA-ELM327_RESPONSE_OFFSET],elm327->eol);
        }
        sim_elm327_start_activity_monitor(elm327);
    }
    if ( ! elm327->responses ) {
        response = strdup("");
    }
    return response;
}