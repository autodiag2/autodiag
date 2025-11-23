#include "libautodiag/sim/elm327/sim_generators.h"
#include "libautodiag/com/serial/elm/elm327/elm327.h"
#include "libautodiag/com/uds/uds.h"

static int dtc_count = 2;
static bool mil_on = true;
static int session_type = UDS_SESSION_DEFAULT;

static bool response(SimECUGenerator *generator, char ** response, final Buffer *binResponse, final Buffer *binRequest) {
    bool responseStatus = true;
    unsigned * seed = generator->context;
    if ( seed == null ) {
        seed = (unsigned*)malloc(sizeof(unsigned));
        *seed = 1;
        generator->context = seed;
    }

    switch(binRequest->buffer[0]) {
        case OBD_SERVICE_SHOW_CURRENT_DATA: {
            bool generic_behaviour = true;
            switch(binRequest->buffer[1]) {
                case 0x01: {
                    Buffer* status = buffer_new();
                    buffer_padding(status, 4, 0x00);
                    status->buffer[0] = dtc_count;
                    status->buffer[0] |= mil_on << 7;
                    buffer_append(binResponse, status);
                    generic_behaviour = false;
                }
            }
            if ( generic_behaviour ) {
                buffer_append(binResponse,buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, seed));
            }
        } break;
        case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA: {
            buffer_append(binResponse,buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, seed));
        } break;
        case OBD_SERVICE_SHOW_DTC: {
            for(int i = 0; i < dtc_count; i++) {
                Buffer *dtc_bin = saej1979_dtc_bin_from_string("P0103");
                buffer_append(binResponse, dtc_bin);
            }
        } break;
        case OBD_SERVICE_PENDING_DTC: case OBD_SERVICE_PERMANENT_DTC: {
            buffer_append(binResponse,buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1, seed));                
        } break;
        case OBD_SERVICE_CLEAR_DTC: {
            mil_on = false;
            dtc_count = 0;
            (*response) = strdup(SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
        } break;
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION: {
            if ( 1 < binRequest->size ) {            
                switch(binRequest->buffer[1]) {
                    case 0x00: {
                        buffer_append(binResponse, buffer_from_ascii_hex("FFFFFFFF"));
                        break;
                    }
                    case 0x01: {
                        buffer_append_byte(binResponse, 0x05);
                        break;
                    }
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_VIN: {
                        buffer_append(binResponse,buffer_from_ascii("VF7RD5FV8FL507366"));                
                        break;
                    }
                    case 0x03: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x04: {
                        buffer_append(binResponse,buffer_new_random_with_seed(16, seed));                
                        break;
                    }
                    case 0x05: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x06: {
                        buffer_append(binResponse,buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x07: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x08: {
                        buffer_append(binResponse,buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x09: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: {
                        final Buffer * name = buffer_from_ascii("ECU citroen C5 X7");
                        buffer_padding(name, 20, 0x00);
                        buffer_append(binResponse, name);
                        break;
                    }
                    case 0x0B: {
                        buffer_append(binResponse,buffer_new_random_with_seed(4, seed));
                        break;
                    }
                }
            } else {
                log_msg(LOG_ERROR, "Missing PID");
            }
        } break;
        case UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL: {
            if ( 1 < binRequest->size ) {
                buffer_append(binResponse, buffer_from_ints( 
                    0x00, 0x19, 0x07, 0xD0 
                ));
            }
        } break;
        case UDS_SERVICE_READ_DATA_BY_IDENTIFIER: {
            if ( 2 < binRequest->size ) {
                if ( (binRequest->size-1) % 2 != 0 ) {
                    responseStatus = false;
                    buffer_append_byte(binResponse, UDS_NRC_INVALID_MESSAGE_LENGTH);
                } else {
                    for(int i = 1; i < (binRequest->size-1); i+=2) {
                        final int did = (binRequest->buffer[i] << 8) | binRequest->buffer[i+1];
                        buffer_append(binResponse, buffer_from_ints(0x00, did));
                    }
                }
            } else {
                responseStatus = false;
                buffer_append_byte(binResponse, UDS_NRC_INVALID_MESSAGE_LENGTH);
            }
        } break;
    }
    return responseStatus;
}
SimECUGenerator* sim_ecu_generator_new_citroen_c5_x7() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->response = SIM_ECU_GENERATOR_RESPONSE_FUNC(response);
    generator->type = strdup("Citroen C5 X7");
    return generator;
}