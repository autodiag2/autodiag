#include "libautodiag/sim/elm327/sim_ecu.h"
#include "libautodiag/sim/elm327/elm327.h"

int SimECU_cmp(SimECU* e1, SimECU* e2) {
    return e1 - e2;
}
AD_LIST_SRC(SimECU)

void list_SimECU_empty(list_SimECU * list) {
    while(0 < list->size) {
        log_msg(LOG_DEBUG, "should free ecu here");
        list_SimECU_remove_at(list, 0);
    }
}

Buffer* sim_ecu_generate_response_header_bin(struct _SimELM327* elm327,SimECU * ecu, byte can28bits_prio) {
    char *protocolSpecificHeader = null;
    Buffer * header = null;
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            char * res;
            asprintf(&res,"%02XDA0000",can28bits_prio);
            header = buffer_from_ascii_hex(res);
            free(res);
            header->buffer[2] = elm327->testerAddress;
            header->buffer[3] = ecu->address;
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            header = buffer_from_ascii_hex("0700");
            header->buffer[header->size-1] = ecu->address;
        } else {
            log_msg(LOG_WARNING, "Missing case here");
        }
    } else {
        header = buffer_from_ascii_hex("410000");
        header->buffer[1] = elm327->testerAddress;
        header->buffer[2] = ecu->address;
    }
    return header;     
}
char * sim_ecu_generate_request_header_bin(struct _SimELM327* elm327,byte source_address, byte can28bits_prio, bool print_spaces) {
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

char * sim_ecu_response_generic(SimELM327 * elm327, SimECU * ecu, Buffer * binRequest) {
    char * response = null;
    if ( 0 == binRequest->size ) {
        log_msg(LOG_ERROR, "No obd/uds data provided");        
        return null;
    }
    final Buffer* binResponse;
    if ( ecu->generator->response_for_python != null ) {
        binResponse = buffer_new();
        ecu->generator->response_for_python(ecu->generator, binRequest, binResponse);
    } else {
        binResponse = ecu->generator->response(ecu->generator, binRequest);
    }
    assert(binResponse != null);
    if ( binRequest->buffer[0] == OBD_SERVICE_CLEAR_DTC ) {
        if ( 0 < binResponse->size ) {
            if ( (binResponse->buffer[0] & OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE) == OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE ) {
                log_msg(LOG_DEBUG, "DTCs cleared request received, replying OK (elm327 style)");
                buffer_recycle(binResponse);
                response = strdup(SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
            }
        }
    }
    if ( 0 < binResponse->size ) {
        assert(response == null);
        bool iso_15765_is_multi_message = false;
        int iso_15765_multi_message_sn = 0;
        int transportLayerMessageDataBytes = 0;
        for(int responseBodyIndex = 0; responseBodyIndex < binResponse->size; responseBodyIndex += transportLayerMessageDataBytes, iso_15765_multi_message_sn += 1) {
            
            final Buffer * responseBodyChunk = buffer_new();
            bool iso_15765_is_multi_message_ff = false;

            if ( responseBodyIndex == 0 ) {
                iso_15765_is_multi_message = 7 < binResponse->size;
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
            transportLayerMessageDataBytes = min(transportLayerMessageDataBytesMax - responseBodyChunk->size, binResponse->size - responseBodyIndex);
            buffer_slice_append(responseBodyChunk, binResponse, responseBodyIndex, transportLayerMessageDataBytes);

            char * space = elm327->printing_of_spaces ? " " : "";
            char *header = "";
            if ( elm327->printing_of_headers ) {
                char *inBuildHeader = "";
                header = sim_ecu_generate_request_header_bin(elm327,ecu->address,ELM327_CAN_28_BITS_DEFAULT_PRIO,elm327->printing_of_spaces);
            } else {
                if ( iso_15765_is_multi_message ) {
                    if ( iso_15765_is_multi_message_ff ) {
                        asprintf(&response, "%03d%s", binResponse->size, elm327->eol);
                    }
                    asprintf(&header,"%d:", iso_15765_multi_message_sn);
                }
            }

            if ( elm327->printing_of_headers || ! elm327->can.auto_format ) {
                if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
                    if ( iso_15765_is_multi_message ) {
                        if ( iso_15765_is_multi_message_ff ) {
                            log_msg(LOG_DEBUG, "reply first frame");
                            int bytesSent = binResponse->size;
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
            asprintf(&tmpResponse, "%s%s%s%s%s", response == null ? "" : response, 
                header, strlen(header) == 0 ? "" : space, 
                elm_ascii_from_bin(elm327->printing_of_spaces, responseBodyChunk), elm327->eol
            );
            free(response);
            response = tmpResponse;
        }
    }
    // leave this commented else when setting custom generators with python there is issues
    // buffer_free(binResponse);
    buffer_free(binRequest);
    return response;
}

SimECU* sim_ecu_emulation_new(byte address) {
    final SimECU* emu = (SimECU*)malloc(sizeof(SimECU));
    emu->sim_ecu_response = (char *(*)(struct _SimELM327 *, struct SimECU *, Buffer *))sim_ecu_response_generic;
    emu->address = address;
    emu->generator = sim_ecu_generator_new_random();
    return emu;
}
