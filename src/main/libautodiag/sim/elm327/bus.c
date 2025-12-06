#include "libautodiag/sim/elm327/bus.h"

/**
 * Generate a header as string for use in the current emulation.
 * Header sent by tester to ECU.
 */
char * sim_ecu_generate_request_header_bin(SimELM327* elm327,byte source_address, byte can28bits_prio, bool print_spaces) {
    char *protocolSpecificHeader = null;
    char * space = print_spaces ? " " : "";
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            asprintf(&protocolSpecificHeader,"%02X%sDA%sF1%s%02hhX", can28bits_prio, space, space, space,source_address);
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            asprintf(&protocolSpecificHeader,"7%02hhX",source_address);
        } else {
            log_msg(LOG_WARNING, "Missing case here");
        }
        if ( elm327->can.extended_addressing ) {
            char *tmp;
            asprintf(&tmp,"%s%s%02hhX", protocolSpecificHeader, space, elm327->can.extended_addressing_target_address);
            free(protocolSpecificHeader);
            protocolSpecificHeader = tmp;
        }
    } else {
        asprintf(&protocolSpecificHeader,"41%s6B%s%02hhX", space,space,source_address);
    }
    return protocolSpecificHeader;     
}

/**
 * Response of the controller to the tester.
 * @return empty buffer in case not addressed to this ECU, null on error, OBD/UDS data on success
 */
static Buffer* data_extract_if_accepted(SimELM327* elm327, SimECU * ecu, Buffer * binRequest, char ** errorCauseReturn) {
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
        assert(0 < dataRequest->size);
        byte pci = dataRequest->buffer[0];
        final int pci_sz_value = pci & 0x0F;
        buffer_left_shift(dataRequest, 1);
        if ( elm327->can.auto_format ) {
            if ( pci_sz_value != dataRequest->size ) {
                log_msg(LOG_ERROR, "Generated pci is different than the actual request size (%d/%d)", pci_sz_value, dataRequest->size);
                assert( pci_sz_value == dataRequest->size);
            }
        } else {
            log_msg(LOG_DEBUG, "For now accepting only single frames");
            assert((pci & 0xF0) == Iso15765SingleFrame);
            if ( dataRequest->size != pci_sz_value ) {
                log_msg(LOG_WARNING, "Single frame pci size does not match");
                *errorCauseReturn = strdup(ELM327ResponseStr[ELM327_RESPONSE_DATA_ERROR_AT_LINE-ELM327_RESPONSE_OFFSET]);
                return null;
            }
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
            final byte pci = dataRequest->size | Iso15765SingleFrame;
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
/**
 * Generate the response header to the tester by the bus.
 */
static Buffer* response_header(SimELM327* elm327,SimECU * ecu, byte can28bits_prio) {
    final Buffer * header = buffer_new();
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            BUFFER_APPEND_BYTES(header, 
                can28bits_prio,
                0xDA,
                elm327->testerAddress,
                ecu->address
            );
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            BUFFER_APPEND_BYTES(header, 
                0x7,
                ecu->address
            );
        } else {
            log_msg(LOG_ERROR, "Missing case here");
            assert( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) || 
                elm327_protocol_is_can_11_bits_id(elm327->protocolRunning)
            );
        }
    } else {
        BUFFER_APPEND_BYTES(header, 
            0x41, 
            elm327->testerAddress,
            ecu->address
        );
    }
    return header;     
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

            char * errorCauseReturn;
            final Buffer * extractedDataRequest = data_extract_if_accepted((SimELM327*)elm327, ecu, binRequest, &errorCauseReturn);
            if ( extractedDataRequest == null ) {
                response = errorCauseReturn;
                break;
            }
            if ( extractedDataRequest->size == 0 ) {
                log_msg(LOG_DEBUG, "Not addressed to this ECU");
                continue;
            }
            char * ecuResponse = null;
            final Buffer * dataResponse = sim_ecu_response(ecu,extractedDataRequest);

            assert(dataResponse != null);
            if ( extractedDataRequest->buffer[0] == OBD_SERVICE_CLEAR_DTC ) {
                if ( 0 < dataResponse->size ) {
                    if ( (dataResponse->buffer[0] & OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE) == OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE ) {
                        log_msg(LOG_DEBUG, "DTCs cleared request received, replying OK (elm327 style)");
                        buffer_recycle(dataResponse);
                        ecuResponse = strdup(SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
                    }
                }
            }
            if ( ecuResponse == null && 0 < dataResponse->size ) {
                assert(ecuResponse == null);
                bool iso_15765_is_multi_message = false;
                int iso_15765_multi_message_sn = 0;
                int transportLayerMessageDataBytes = 0;
                for(int responseBodyIndex = 0; responseBodyIndex < dataResponse->size; responseBodyIndex += transportLayerMessageDataBytes, iso_15765_multi_message_sn += 1) {
                    
                    final Buffer * responseBodyChunk = buffer_new();
                    bool iso_15765_is_multi_message_ff = false;

                    if ( responseBodyIndex == 0 ) {
                        iso_15765_is_multi_message = 7 < dataResponse->size;
                        if ( iso_15765_is_multi_message ) {
                            iso_15765_is_multi_message_ff = true;
                        }
                    }

                    int transportLayerMessageDataBytesMax = 7;
                    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
                        if ( iso_15765_is_multi_message ) {
                            if ( iso_15765_is_multi_message_ff ) {
                                transportLayerMessageDataBytesMax = 6;
                            }
                        }
                    }
                    transportLayerMessageDataBytes = min(transportLayerMessageDataBytesMax - responseBodyChunk->size, dataResponse->size - responseBodyIndex);
                    buffer_slice_append(responseBodyChunk, dataResponse, responseBodyIndex, transportLayerMessageDataBytes);

                    char * space = elm327->printing_of_spaces ? " " : "";
                    char *header = "";
                    if ( elm327->printing_of_headers ) {
                        char *inBuildHeader = "";
                        header = sim_ecu_generate_request_header_bin(elm327,ecu->address,ELM327_CAN_28_BITS_DEFAULT_PRIO,elm327->printing_of_spaces);
                    } else {
                        if ( iso_15765_is_multi_message ) {
                            if ( iso_15765_is_multi_message_ff ) {
                                asprintf(&ecuResponse, "%03d%s", dataResponse->size, elm327->eol);
                            }
                            asprintf(&header,"%d:", iso_15765_multi_message_sn);
                        }
                    }

                    if ( elm327->printing_of_headers || ! elm327->can.auto_format ) {
                        if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
                            if ( iso_15765_is_multi_message ) {
                                if ( iso_15765_is_multi_message_ff ) {
                                    log_msg(LOG_DEBUG, "reply first frame");
                                    int bytesSent = dataResponse->size;
                                    int dl11_8 = (bytesSent & 0x0F00) >> 8;
                                    final byte pci = Iso15765FirstFrame << 4 | dl11_8;
                                    final byte dl7_0 = bytesSent & 0xFF;
                                    buffer_prepend_byte(responseBodyChunk, dl7_0);
                                    buffer_prepend_byte(responseBodyChunk, pci);
                                } else {
                                    log_msg(LOG_DEBUG, "reply consecutive frame");
                                    final byte pci = Iso15765ConsecutiveFrame << 4 | iso_15765_multi_message_sn;
                                    buffer_prepend_byte(responseBodyChunk, pci);
                                }
                            } else {
                                log_msg(LOG_DEBUG, "reply as single frame");
                                final byte pci = Iso15765SingleFrame | responseBodyChunk->size;
                                buffer_prepend_byte(responseBodyChunk, pci);
                            }
                        }
                    }

                    char *tmpResponse;
                    asprintf(&tmpResponse, "%s%s%s%s%s", ecuResponse == null ? "" : ecuResponse, 
                        header, strlen(header) == 0 ? "" : space, 
                        elm_ascii_from_bin(elm327->printing_of_spaces, responseBodyChunk), elm327->eol
                    );
                    free(ecuResponse);
                    ecuResponse = tmpResponse;
                }
            }
            // end of response data

            Buffer * response_header_bin = response_header(elm327,ecu,ELM327_CAN_28_BITS_DEFAULT_PRIO);
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
                        free(ecuResponse);
                        ecuResponse = null;
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

            if ( ecuResponse != null ) {
                char * tmpResponseResult;
                asprintf(&tmpResponseResult,"%s%s%s",response==null?"":response,ecuResponse,i+1<LIST_SIM_ECU(elm327->ecus)->size ? elm327->eol : "");
                ecuResponse = null; // we do not free to compat with python
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