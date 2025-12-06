#include "libautodiag/sim/ecu/generator.h"

static Buffer * response(SimECUGenerator *generator, final Buffer *binRequest) {
    assert(generator->context != null);
    unsigned * seed = generator->context;
    final Buffer *binResponse = buffer_new();
    sim_ecu_generator_fill_success(binResponse, binRequest);

    switch(binRequest->buffer[0]) {

        case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA:
        case OBD_SERVICE_SHOW_CURRENT_DATA: {
            buffer_append(binResponse,
                buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, seed));
        } break;

        case OBD_SERVICE_PENDING_DTC:
        case OBD_SERVICE_PERMANENT_DTC:
        case OBD_SERVICE_SHOW_DTC: {
            buffer_append(binResponse,
                buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1, seed));
        } break;

        case OBD_SERVICE_CLEAR_DTC: {
            log_msg(LOG_DEBUG, "Clearing DTCs");
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
                        buffer_append(binResponse,
                            buffer_new_random_with_seed(17, seed));
                        break;
                    }
                    case 0x03: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x04: {
                        buffer_append(binResponse,
                            buffer_new_random_with_seed(16, seed));
                        break;
                    }
                    case 0x05: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x06: {
                        buffer_append(binResponse,
                            buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x07: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x08: {
                        buffer_append(binResponse,
                            buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x09: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: {
                        final Buffer * name = buffer_from_ascii("ECU random");
                        buffer_padding(name, 20, 0x00);
                        buffer_append(binResponse, name);
                        break;
                    }
                    case 0x0B: {
                        buffer_append(binResponse,
                            buffer_new_random_with_seed(4, seed));
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

SimECUGenerator* sim_ecu_generator_new_random() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->response = SIM_ECU_GENERATOR_RESPONSE(response);
    generator->type = strdup("random");
    unsigned * seed = (unsigned*)malloc(sizeof(unsigned));
    *seed = 1;
    generator->context = seed;
    return generator;
}
