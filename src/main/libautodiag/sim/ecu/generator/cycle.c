#include "libautodiag/sim/ecu/generator.h"

typedef struct {
    byte cycle_percent[256][256];
} GState;

static Buffer * response(SimECUGenerator *generator, final Buffer *binRequest) {
    assert(generator->context != null);
    unsigned gears = *((unsigned*)generator->context);
    GState * state = (GState*)generator->state;
    final Buffer *binResponse = buffer_new();
    sim_ecu_generator_fill_success(binResponse, binRequest);

    switch(binRequest->buffer[0]) {
        case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA:
        case OBD_SERVICE_SHOW_CURRENT_DATA: {
            buffer_append(binResponse,
                buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2,
                state->cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
        } break;

        case OBD_SERVICE_PENDING_DTC:
        case OBD_SERVICE_PERMANENT_DTC:
        case OBD_SERVICE_SHOW_DTC: {
            buffer_append(binResponse,
                buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1,
                state->cycle_percent[binRequest->buffer[0]][0]));
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
                            buffer_new_cycle(17,
                            state->cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
                        break;
                    }
                    case 0x03: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x04: {
                        buffer_append(binResponse,
                            buffer_new_cycle(16,
                            state->cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
                        break;
                    }
                    case 0x05: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x06: {
                        buffer_append(binResponse,
                            buffer_new_cycle(4,
                            state->cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
                        break;
                    }
                    case 0x07: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x08: {
                        buffer_append(binResponse,
                            buffer_new_cycle(4,
                            state->cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
                        break;
                    }
                    case 0x09: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: {
                        final Buffer * name = buffer_from_ascii("ECU cycle");
                        buffer_padding(name, 20, 0x00);
                        buffer_append(binResponse, name);
                        break;
                    }
                    case 0x0B: {
                        buffer_append(binResponse,
                            buffer_new_cycle(4,
                            state->cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
                        break;
                    }
                }
            } else {
                log_msg(LOG_ERROR, "Missing PID");
            }
        } break;
    }

    int service_id = binRequest->buffer[0];
    int pid = 1 < binRequest->size ? binRequest->buffer[1] : 0;
    state->cycle_percent[service_id][pid] += (100/gears);
    state->cycle_percent[service_id][pid] %= 100;
    return binResponse;
}
static char * context_to_string(SimECUGenerator * this) {
    unsigned * gears = this->context;
    return gprintf("%d", *gears);
}
static bool context_load_from_string(SimECUGenerator * this, char * context) {
    unsigned * gears = this->context;
    return sscanf(context, "%d", gears) == 1;
}
SimECUGenerator* sim_ecu_generator_new_cycle() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->response = SIM_ECU_GENERATOR_RESPONSE(response);
    generator->context_load_from_string = SIM_ECU_GENERATOR_CONTEXT_LOAD_FROM_STRING(context_load_from_string);
    generator->context_to_string = SIM_ECU_GENERATOR_CONTEXT_TO_STRING(context_to_string);
    generator->type = strdup("cycle");
    GState * state = (GState*)calloc(1, sizeof(GState));
    generator->state = (void*)state;
    unsigned * gears = (unsigned*)malloc(sizeof(unsigned));
    *gears = 10;
    generator->context = gears;
    return generator;
}