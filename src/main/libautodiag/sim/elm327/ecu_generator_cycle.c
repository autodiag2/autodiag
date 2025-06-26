#include "libautodiag/sim/elm327/sim_generators.h"
#include "libautodiag/com/serial/elm/elm327/elm327.h"

int sim_ecu_generator_cycle_percent[0xFF][0xFF] = {0};
void sim_ecu_generator_cycle_iterate(int service_id, int pid, unsigned gears) {
    sim_ecu_generator_cycle_percent[service_id][pid] += (100/gears);
    sim_ecu_generator_cycle_percent[service_id][pid] %= 100;
}
void sim_ecu_generator_response_cycle(SimECUGenerator *generator, char ** response, final Buffer *binResponse, final Buffer *binRequest) {
    unsigned gears = 10;
    if ( generator->context != null ) {
        gears = *((unsigned*)generator->context);
    }
    switch(binRequest->buffer[0]) {
        case 0x02: case 0x01: {
            buffer_append(binResponse,buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, sim_ecu_generator_cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
        } break;
        case 0x07: case 0x0A: case 0x03: {
            buffer_append(binResponse,buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1, sim_ecu_generator_cycle_percent[binRequest->buffer[0]][0]));                
        } break;
        case 0x04: {
            (*response) = strdup(SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
        } break;
        case 0x09: {
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
                    case 0x02: {
                        buffer_append(binResponse,buffer_new_cycle(17, sim_ecu_generator_cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));                
                        break;
                    }
                    case 0x03: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x04: {
                        buffer_append(binResponse,buffer_new_cycle(16, sim_ecu_generator_cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));                
                        break;
                    }
                    case 0x05: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x06: {
                        buffer_append(binResponse,buffer_new_cycle(4, sim_ecu_generator_cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
                        break;
                    }
                    case 0x07: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x08: {
                        buffer_append(binResponse,buffer_new_cycle(4, sim_ecu_generator_cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
                        break;
                    }
                    case 0x09: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x0A: {
                        final Buffer * name = buffer_from_ascii("TEST");
                        buffer_padding(name, 20, 0x00);
                        buffer_append(binResponse, name);
                        break;
                    }
                    case 0x0B: {
                        buffer_append(binResponse,buffer_new_cycle(4, sim_ecu_generator_cycle_percent[binRequest->buffer[0]][binRequest->buffer[1]]));
                        break;
                    }
                }
            } else {
                log_msg(LOG_ERROR, "Missing PID");
            }
        } break;
    }
    sim_ecu_generator_cycle_iterate(binRequest->buffer[0], 1 < binRequest->size ? binRequest->buffer[1] : 0, gears);
}

SimECUGenerator* sim_ecu_generator_new_cycle() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->response = SIM_ECU_GENERATOR_RESPONSE_FUNC(sim_ecu_generator_response_cycle);
    generator->type = strdup("cycle");
    return generator;
}