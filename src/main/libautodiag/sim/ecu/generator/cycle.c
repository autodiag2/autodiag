#include "libautodiag/sim/ecu/generator.h"

static int cycle_percent[0xFF][0xFF] = {0};
static void cycle_iterate(int service_id, int pid, unsigned gears) {
    cycle_percent[service_id][pid] += (100/gears);
    cycle_percent[service_id][pid] %= 100;
}

static Buffer * response(SimECUGenerator *generator, final Buffer *binRequest) {
    unsigned gears = 10;
    if ( generator->context != null ) {
        gears = *((unsigned*)generator->context);
    }
    final Buffer *binResponse = buffer_new();
    sim_ecu_generator_fill_success(binResponse, binRequest);

    switch(binRequest->buffer[0]) {
        case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA:
        case OBD_SERVICE_SHOW_CURRENT_DATA: {
            buffer_append(binResponse,
                buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2,
                cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
        } break;

        case OBD_SERVICE_PENDING_DTC:
        case OBD_SERVICE_PERMANENT_DTC:
        case OBD_SERVICE_SHOW_DTC: {
            buffer_append(binResponse,
                buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1,
                cycle_percent[binRequest->buffer[0]][0]));
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
                            cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
                        break;
                    }
                    case 0x03: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x04: {
                        buffer_append(binResponse,
                            buffer_new_cycle(16,
                            cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
                        break;
                    }
                    case 0x05: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x06: {
                        buffer_append(binResponse,
                            buffer_new_cycle(4,
                            cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
                        break;
                    }
                    case 0x07: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x08: {
                        buffer_append(binResponse,
                            buffer_new_cycle(4,
                            cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
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
                            cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
                        break;
                    }
                }
            } else {
                log_msg(LOG_ERROR, "Missing PID");
            }
        } break;
    }

    cycle_iterate(binRequest->buffer[0],
        1 < binRequest->size ? binRequest->buffer[1] : 0,
        gears);
    return binResponse;
}

SimECUGenerator* sim_ecu_generator_new_cycle() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->response = SIM_ECU_GENERATOR_RESPONSE_FUNC(response);
    generator->type = strdup("cycle");
    return generator;
}