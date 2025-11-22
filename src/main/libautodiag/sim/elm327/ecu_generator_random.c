#include "libautodiag/sim/elm327/sim_generators.h"
#include "libautodiag/com/serial/elm/elm327/elm327.h"

static void response(SimECUGenerator *generator, char ** response, final Buffer *binResponse, final Buffer *binRequest) {
    unsigned * seed = generator->context;
    if ( seed == null ) {
        seed = (unsigned*)malloc(sizeof(unsigned));
        *seed = 1;
        generator->context = seed;
    }

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
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_E_VIN: {
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
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_E_ECU_NAME: {
                        final Buffer * name = buffer_from_ascii("TEST");
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
}

SimECUGenerator* sim_ecu_generator_new_random() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->response = SIM_ECU_GENERATOR_RESPONSE_FUNC(response);
    generator->type = strdup("random");
    return generator;
}
