#include "libautodiag/sim/ecu/generator.h"

static Buffer * saej1979_response_dtcs(SimECUGenerator *generator, int service_id) {
    unsigned * seed = generator->context;
    return ad_buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1, seed);
}
static Buffer * saej1979_response_pid(SimECUGenerator *generator, final byte pid, int frameNumber) {
    Buffer * binResponse = ad_buffer_new();
    // Should append only bytes according to the PID, but for simplicity we just append random data
    switch(pid) {
        case 0xC0:
        case 0xA0:
        case 0x80:
        case 0x60:
        case 0x40:
        case 0x20:
        case 0x00: {
            ad_buffer_append_melt(binResponse, ad_buffer_from_ascii_hex("FFFFFFFFFF"));
        } break;
        default: {
            unsigned * seed = generator->context;
            ad_buffer_append_melt(binResponse,
                ad_buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, seed));
        } break;
    }
    return binResponse;
}
static Buffer * response(SimECUGenerator *generator, final Buffer *binRequest) {
    assert(generator->context != null);
    unsigned * seed = generator->context;
    final Buffer *binResponse = ad_buffer_new();
    if ( binRequest->size == 0 ) {
        return binResponse;
    }
    if ( ! sim_ecu_generator_fill_success(binResponse, binRequest) ) {
        return ad_buffer_new();
    }

    switch(binRequest->buffer[0]) {
        case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA:
        case OBD_SERVICE_SHOW_CURRENT_DATA:
            return generator->saej1979_response_pids(generator, binRequest);

        case OBD_SERVICE_PENDING_DTC:
        case OBD_SERVICE_PERMANENT_DTC:
        case OBD_SERVICE_SHOW_DTC:
            return generator->saej1979_response_dtcs_wrapper(generator, binRequest->buffer[0]);

        case OBD_SERVICE_CLEAR_DTC: {
            log_msg(LOG_DEBUG, "Clearing DTCs");
        } break;

        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION: {
            if ( 1 < binRequest->size ) {
                switch(binRequest->buffer[1]) {
                    case 0x00: {
                        ad_buffer_append_melt(binResponse, ad_buffer_from_ascii_hex("FFFFFFFFFF"));
                        break;
                    }
                    case 0x01: {
                        ad_buffer_append_byte(binResponse, 0x05);
                        break;
                    }
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_VIN: {
                        ad_buffer_append_melt(binResponse,
                            ad_buffer_new_random_with_seed(17, seed));
                        break;
                    }
                    case 0x03: {
                        ad_buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x04: {
                        ad_buffer_append_melt(binResponse,
                            ad_buffer_new_random_with_seed(16, seed));
                        break;
                    }
                    case 0x05: {
                        ad_buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x06: {
                        ad_buffer_append_melt(binResponse,
                            ad_buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x07: {
                        ad_buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x08: {
                        ad_buffer_append_melt(binResponse,
                            ad_buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x09: {
                        ad_buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: {
                        final Buffer * name = ad_buffer_from_ascii("ECU random");
                        ad_buffer_padding(name, 20, 0x00);
                        ad_buffer_append_melt(binResponse, name);
                        break;
                    }
                    case 0x0B: {
                        ad_buffer_append_melt(binResponse,
                            ad_buffer_new_random_with_seed(4, seed));
                        break;
                    }
                }
            } else {
                log_msg(LOG_ERROR, "Missing PID");
            }
        } break;
    }
    return binResponse;
}
static char * context_to_string(SimECUGenerator * this) {
    unsigned * seed = this->context;
    return gprintf("%d", *seed);
}
static bool context_load_from_string(SimECUGenerator * this, char * context) {
    unsigned * seed = this->context;
    return sscanf(context, "%d", seed) == 1;
}

SimECUGenerator* sim_ecu_generator_new_random() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->response = SIM_ECU_GENERATOR_RESPONSE(response);
    generator->context_load_from_string = SIM_ECU_GENERATOR_CONTEXT_LOAD_FROM_STRING(context_load_from_string);
    generator->context_to_string = SIM_ECU_GENERATOR_CONTEXT_TO_STRING(context_to_string);
    generator->type = strdup("random");
    generator->flavour.is_Iso15765_4 = 0;
    generator->saej1979_response_pid = saej1979_response_pid;
    generator->saej1979_response_dtcs = saej1979_response_dtcs;
    unsigned * seed = (unsigned*)malloc(sizeof(unsigned));
    *seed = 1;
    generator->context = seed;
    return generator;
}
