#include "libautodiag/sim/elm327/sim.h"
#include "libautodiag/sim/elm327/elm327.h"

LIST_DEFINE_MEMBERS_SYM_AUTO(ECUEmulation)

Buffer* ecu_sim_generate_header_bin(struct _ELM327emulation* elm327,ECUEmulation * ecu, byte can28bits_prio) {
    char *protocolSpecificHeader = null;
    Buffer * header = null;
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            char * res;
            asprintf(&res,"%02XDA0000",can28bits_prio);
            header = buffer_from_ascii_hex(res);
            free(res);
            header->buffer[3] = elm327->testerAddress;
            header->buffer[4] = ecu->address;
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
char * ecu_sim_generate_obd_header(struct _ELM327emulation* elm327,byte source_address, byte can28bits_prio, bool print_spaces) {
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

char * ecu_saej1979_sim_response(ECUEmulation * ecu, ELM327emulation * elm327, char * obd_query_str, bool hasSpaces) {
    char * response = null;
    Buffer* obd_query_bin = buffer_new();
    Buffer* responseOBDdataBin = buffer_new();
    char * end_ptr = strstr(obd_query_str,elm327->eol);

    int szToRemove = 0;
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            szToRemove = (4 + 1) * (2 + hasSpaces);
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            szToRemove = (3 + 2) + hasSpaces * 2;
        } else {
            log_msg(LOG_WARNING, "Missing case here");
        }
        if ( elm327->can.extended_addressing ) {
            szToRemove += 2 + hasSpaces;
        }
        if ( strlen(obd_query_str) <= szToRemove ) {
            log_msg(LOG_ERROR, "Can auto formatting is disabled, but seem header not provided");
            return null;
        }
    } else {
        szToRemove = 3 * (2 + hasSpaces);
    }
    obd_query_str = obd_query_str + szToRemove;

    elm_ascii_to_bin_internal(hasSpaces, obd_query_bin, obd_query_str, end_ptr == null ? obd_query_str + strlen(obd_query_str): end_ptr);
    if ( ! elm327->can.auto_format ) {
        byte pci = obd_query_bin->buffer[0];
        assert((pci & 0xF0) == Iso15765SingleFrame);
        int sz = pci & 0x0F;
        if ( obd_query_bin->size != (sz + 1) ) {
            log_msg(LOG_WARNING, "Single frame pci size does not match");
            return strdup(ELM327ResponseStr[ELM327_RESPONSE_DATA_ERROR_AT_LINE-ELM327_RESPONSE_OFFSET]);
        }
    }
    if ( 0 == obd_query_bin->size ) {
        log_msg(LOG_ERROR, "No obd data provided");        
        return null;
    }
    ecu->generator->obd_sim_response(ecu->generator, &response, responseOBDdataBin, obd_query_bin);
    if ( 0 < responseOBDdataBin->size ) {
        assert(response == null);
        bool iso_15765_is_multi_message = false;
        int iso_15765_multi_message_sn = 0;
        int obdMessageDataBytes = 0;
        for(int responseBodyIndex = 0; responseBodyIndex < responseOBDdataBin->size; responseBodyIndex += obdMessageDataBytes, iso_15765_multi_message_sn += 1) {
            
            final Buffer * responseBodyChunk = buffer_new();
            bool iso_15765_is_multi_message_ff = false;
            bool hasPid = false;
            switch(obd_query_bin->buffer[0]) {
                case 0x01: case 0x02: case 0x09: hasPid = true; break;
            }

            if ( responseBodyIndex == 0 ) {
                buffer_append_byte(responseBodyChunk, obd_query_bin->buffer[0] | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE);
                if ( hasPid ) {
                    buffer_append_byte(responseBodyChunk, obd_query_bin->buffer[1]);
                }
                iso_15765_is_multi_message = 7 < responseOBDdataBin->size;
                if ( iso_15765_is_multi_message ) {
                    iso_15765_is_multi_message_ff = true;
                }
            }

            int obdMessageDataBytesMax = 7;
            if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
                if ( iso_15765_is_multi_message ) {
                    if ( iso_15765_is_multi_message_ff ) {
                        obdMessageDataBytesMax = 6;
                    }
                }
            }
            obdMessageDataBytes = min(obdMessageDataBytesMax - responseBodyChunk->size, responseOBDdataBin->size - responseBodyIndex);
            buffer_slice(responseBodyChunk, responseOBDdataBin, responseBodyIndex, obdMessageDataBytes);

            char * space = elm327->printing_of_spaces ? " " : "";
            char *header = "";
            if ( elm327->printing_of_headers ) {
                char *inBuildHeader = "";
                header = ecu_sim_generate_obd_header(elm327,ecu->address,ELM327_CAN_28_BITS_DEFAULT_PRIO,elm327->printing_of_spaces);
            } else {
                if ( iso_15765_is_multi_message ) {
                    if ( iso_15765_is_multi_message_ff ) {
                        int extra_size = 1 + hasPid;
                        asprintf(&response, "%03d%s", extra_size + responseOBDdataBin->size, elm327->eol);
                    }
                    asprintf(&header,"%d:", iso_15765_multi_message_sn);
                }
            }

            if ( elm327->printing_of_headers || ! elm327->can.auto_format ) {
                if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
                    if ( iso_15765_is_multi_message ) {
                        if ( iso_15765_is_multi_message_ff ) {
                            log_msg(LOG_DEBUG, "reply first frame");
                            int bytesSent = responseOBDdataBin->size + 1 + hasPid;
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
    buffer_free(responseOBDdataBin);
    buffer_free(obd_query_bin);
    return response;
}

ECUEmulation* ecu_emulation_new(byte address) {
    final ECUEmulation* emu = (ECUEmulation*)malloc(sizeof(ECUEmulation));
    emu->saej1979_sim_response = (char *(*)(struct ECUEmulation *, struct _ELM327emulation *, char *, int))ecu_saej1979_sim_response;
    emu->address = address;
    emu->generator = sim_ecu_generator_new_random();
    return emu;
}
