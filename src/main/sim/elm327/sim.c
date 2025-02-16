#include "sim/elm327/sim.h"

LIST_DEFINE_MEMBERS_SYM_AUTO(ECUEmulation)

Buffer* ecu_sim_generate_header_bin(ELM327emulation* elm327,ECUEmulation * ecu, byte can28bits_prio) {
    char *protocolSpecificHeader = null;
    Buffer * header = null;
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            char * res;
            asprintf(&res,"%02XDAF100",can28bits_prio);
            header = ascii_to_bin_buffer(res);
            free(res);
            header->buffer[header->size-1] = ecu->address;
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            header = ascii_to_bin_buffer("0700");
            header->buffer[header->size-1] = ecu->address;
        } else {
            log_msg(LOG_WARNING, "Missing case here");
        }
    } else {
        header = ascii_to_bin_buffer("416B00");
        header->buffer[header->size-1] = ecu->address;
    }
    return header;     
}
char * ecu_sim_generate_obd_header(ELM327emulation* elm327,byte source_address, byte can28bits_prio) {
    char *protocolSpecificHeader = null;
    char * space = elm327->printing_of_spaces ? " " : "";
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            asprintf(&protocolSpecificHeader,"%02X%sDA%sF1%s%02hhX", can28bits_prio, space, space, space,source_address);
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            asprintf(&protocolSpecificHeader,"7%02hhX",source_address);
        } else {
            log_msg(LOG_WARNING, "Missing case here");
        }
    } else {
        asprintf(&protocolSpecificHeader,"41%s6B%s%02hhX", space,space,source_address);
    }
    return protocolSpecificHeader;     
}

char * ecu_saej1979_sim_response(ECUEmulation * ecu, ELM327emulation * elm327, char * obd_query_str, bool hasSpaces) {
    char * response = null;
    Buffer* bin = buffer_new();
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
        if ( strlen(obd_query_str) <= szToRemove ) {
            log_msg(LOG_ERROR, "Can auto formatting is disabled, but seem header not provided");
            return null;
        }
    } else {
        szToRemove = 3 * (2 + hasSpaces);
    }
    obd_query_str = obd_query_str + szToRemove;

    elm_ascii_to_bin_internal(hasSpaces, bin, obd_query_str, end_ptr == null ? obd_query_str + strlen(obd_query_str): end_ptr);
    
    if ( 0 == bin->size ) {
        log_msg(LOG_ERROR, "No obd data provided");        
        return null;
    }

    switch(bin->buffer[0]) {
        case 0x02: case 0x01: {
            buffer_append(responseOBDdataBin,buffer_new_random(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1));
        } break;
        case 0x07: case 0x0A: case 0x03: {
            buffer_append(responseOBDdataBin,buffer_new_random(ISO_15765_SINGLE_FRAME_DATA_BYTES));                
        } break;
        case 0x04: {
            response = strdup(SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
        } break;
        case 0x09: {
            if ( 1 < bin->size ) {            
                switch(bin->buffer[1]) {
                    case 0x0A:
                        // ECU name : 'TEST'
                        buffer_append(responseOBDdataBin, ascii_to_bin_buffer("5445535400"));
                        break;
                }
            } else {
                log_msg(LOG_ERROR, "Missing PID");
            }
        } break;
    }
    if ( 0 < responseOBDdataBin->size ) {
        char *header = "";
        if ( elm327->printing_of_headers ) {
            char * protocolSpecificHeader = ecu_sim_generate_obd_header(elm327,ecu->address,ELM327_CAN_28_BITS_DEFAULT_PRIO);
            char * space = elm327->printing_of_spaces ? " " : "";
            
            bool hasPid = false;
            switch(bin->buffer[0]) {
                case 0x01: case 0x02: case 0x09: hasPid = true; break;
            }
            if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
                assert(responseOBDdataBin->size <= ISO_15765_SINGLE_FRAME_DATA_BYTES - hasPid);
                if ( hasPid ) {
                    asprintf(&header, "%s%s%02X%s%02X%s%02X%s", protocolSpecificHeader, space, responseOBDdataBin->size + 1 + hasPid, space, bin->buffer[0] | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE, space, bin->buffer[1], space);
                } else {
                    asprintf(&header, "%s%s%02X%s%02X%s", protocolSpecificHeader, space, responseOBDdataBin->size + 1 + hasPid, space, bin->buffer[0] | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE, space);
                }
            } else {
                if ( hasPid ) {
                    asprintf(&header, "%s%s%02X%s%02X%s", protocolSpecificHeader, space, bin->buffer[0] | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE, space, bin->buffer[1], space);
                } else {
                    asprintf(&header, "%s%s%02X%s", protocolSpecificHeader, space, bin->buffer[0] | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE, space);
                }
            }
        }
        asprintf(&response, "%s%s", header, elm_ascii_from_bin(elm327->printing_of_spaces, responseOBDdataBin));
    }
    buffer_free(responseOBDdataBin);
    buffer_free(bin);
    return response;
}

ECUEmulation* ecu_emulation_new(byte address) {
    ECUEmulation* emu = (ECUEmulation*)malloc(sizeof(ECUEmulation));
    emu->saej1979_sim_response = (char * (*)(_ECUEmulation *, _ELM327emulation *, char *,bool))ecu_saej1979_sim_response;
    emu->address = address;
    return emu;
}
