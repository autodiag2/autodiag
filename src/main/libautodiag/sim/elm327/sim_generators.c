#include "libautodiag/sim/elm327/sim_generators.h"
#include "libautodiag/com/serial/elm/elm327/elm327.h"

SimECUGenerator * sim_ecu_generator_new() {
    SimECUGenerator * generator = (SimECUGenerator*)malloc(sizeof(SimECUGenerator));
    generator->context = null;
    generator->sim_ecu_generator_response = null;
    generator->type = null;
    return generator;
}

int sim_ecu_generator_cycle_percent[0xFF][0xFF] = {0};
void sim_ecu_generator_cycle_iterate(int service_id, int pid, unsigned gears) {
    sim_ecu_generator_cycle_percent[service_id][pid] += (100/gears);
    sim_ecu_generator_cycle_percent[service_id][pid] %= 100;
}
void sim_ecu_generator_cycle(SimECUGenerator *generator, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin) {
    unsigned gears = 10;
    if ( generator->context != null ) {
        gears = *((unsigned*)generator->context);
    }
    switch(obd_query_bin->buffer[0]) {
        case 0x02: case 0x01: {
            buffer_append(responseOBDdataBin,buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, sim_ecu_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));
        } break;
        case 0x07: case 0x0A: case 0x03: {
            buffer_append(responseOBDdataBin,buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1, sim_ecu_generator_cycle_percent[obd_query_bin->buffer[0]][0]));                
        } break;
        case 0x04: {
            (*response) = strdup(SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
        } break;
        case 0x09: {
            if ( 1 < obd_query_bin->size ) {            
                switch(obd_query_bin->buffer[1]) {
                    case 0x00: {
                        buffer_append(responseOBDdataBin, buffer_from_ascii_hex("FFFFFFFF"));
                        break;
                    }
                    case 0x01: {
                        buffer_append_byte(responseOBDdataBin, 0x05);
                        break;
                    }
                    case 0x02: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(17, sim_ecu_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));                
                        break;
                    }
                    case 0x03: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x04: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(16, sim_ecu_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));                
                        break;
                    }
                    case 0x05: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x06: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(4, sim_ecu_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));
                        break;
                    }
                    case 0x07: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x08: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(4, sim_ecu_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));
                        break;
                    }
                    case 0x09: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x0A: {
                        final Buffer * name = buffer_from_ascii("TEST");
                        buffer_padding(name, 20, 0x00);
                        buffer_append(responseOBDdataBin, name);
                        break;
                    }
                    case 0x0B: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(4, sim_ecu_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));
                        break;
                    }
                }
            } else {
                log_msg(LOG_ERROR, "Missing PID");
            }
        } break;
    }
    sim_ecu_generator_cycle_iterate(obd_query_bin->buffer[0], 1 < obd_query_bin->size ? obd_query_bin->buffer[1] : 0, gears);
}

SimECUGenerator* sim_ecu_generator_new_cycle() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->sim_ecu_generator_response = SIM_ECU_GENERATOR_RESPONSE_FUNC(sim_ecu_generator_cycle);
    generator->type = strdup("cycle");
    return generator;
}

void sim_ecu_generator_random(SimECUGenerator *generator, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin) {
    unsigned * seed = generator->context;
    if ( seed == null ) {
        seed = (unsigned*)malloc(sizeof(unsigned));
        *seed = 1;
        generator->context = seed;
    }
    switch(obd_query_bin->buffer[0]) {
        case 0x02: case 0x01: {
            buffer_append(responseOBDdataBin,buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, seed));
        } break;
        case 0x07: case 0x0A: case 0x03: {
            buffer_append(responseOBDdataBin,buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1, seed));                
        } break;
        case 0x04: {
            (*response) = strdup(SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
        } break;
        case 0x09: {
            if ( 1 < obd_query_bin->size ) {            
                switch(obd_query_bin->buffer[1]) {
                    case 0x00: {
                        buffer_append(responseOBDdataBin, buffer_from_ascii_hex("FFFFFFFF"));
                        break;
                    }
                    case 0x01: {
                        buffer_append_byte(responseOBDdataBin, 0x05);
                        break;
                    }
                    case 0x02: {
                        buffer_append(responseOBDdataBin,buffer_new_random_with_seed(17, seed));                
                        break;
                    }
                    case 0x03: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x04: {
                        buffer_append(responseOBDdataBin,buffer_new_random_with_seed(16, seed));                
                        break;
                    }
                    case 0x05: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x06: {
                        buffer_append(responseOBDdataBin,buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x07: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x08: {
                        buffer_append(responseOBDdataBin,buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x09: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x0A: {
                        final Buffer * name = buffer_from_ascii("TEST");
                        buffer_padding(name, 20, 0x00);
                        buffer_append(responseOBDdataBin, name);
                        break;
                    }
                    case 0x0B: {
                        buffer_append(responseOBDdataBin,buffer_new_random_with_seed(4, seed));
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
    generator->sim_ecu_generator_response = SIM_ECU_GENERATOR_RESPONSE_FUNC(sim_ecu_generator_random);
    generator->type = strdup("random");
    return generator;
}