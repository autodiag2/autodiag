#include "sim/elm327/sim_generators.h"

int ecu_saej1979_sim_generator_cycle_percent[0xFF][0xFF] = {0};
void ecu_saej1979_sim_generator_cycle_iterate(int service_id, int pid) {
    ecu_saej1979_sim_generator_cycle_percent[service_id][pid] += 10;
    ecu_saej1979_sim_generator_cycle_percent[service_id][pid] %= 100;
}
void ecu_saej1979_sim_generator_cycle(ECUEmulationGenerator *generator, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin) {
    switch(obd_query_bin->buffer[0]) {
        case 0x02: case 0x01: {
            buffer_append(responseOBDdataBin,buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));
        } break;
        case 0x07: case 0x0A: case 0x03: {
            buffer_append(responseOBDdataBin,buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][0]));                
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
                        buffer_append(responseOBDdataBin,buffer_new_cycle(17, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));                
                        break;
                    }
                    case 0x03: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x04: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(16, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));                
                        break;
                    }
                    case 0x05: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x06: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(4, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));
                        break;
                    }
                    case 0x07: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x08: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(4, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));
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
                        buffer_append(responseOBDdataBin,buffer_new_cycle(4, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));
                        break;
                    }
                }
            } else {
                log_msg(LOG_ERROR, "Missing PID");
            }
        } break;
    }
    ecu_saej1979_sim_generator_cycle_iterate(obd_query_bin->buffer[0], 1 < obd_query_bin->size ? obd_query_bin->buffer[1] : 0);
}

void ecu_saej1979_sim_generator_random(ECUEmulationGenerator *generator, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin) {
    switch(obd_query_bin->buffer[0]) {
        case 0x02: case 0x01: {
            buffer_append(responseOBDdataBin,buffer_new_random(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2));
        } break;
        case 0x07: case 0x0A: case 0x03: {
            buffer_append(responseOBDdataBin,buffer_new_random(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1));                
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
                        buffer_append(responseOBDdataBin,buffer_new_random(17));                
                        break;
                    }
                    case 0x03: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x04: {
                        buffer_append(responseOBDdataBin,buffer_new_random(16));                
                        break;
                    }
                    case 0x05: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x06: {
                        buffer_append(responseOBDdataBin,buffer_new_random(4));
                        break;
                    }
                    case 0x07: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x08: {
                        buffer_append(responseOBDdataBin,buffer_new_random(4));
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
                        buffer_append(responseOBDdataBin,buffer_new_random(4));
                        break;
                    }
                }
            } else {
                log_msg(LOG_ERROR, "Missing PID");
            }
        } break;
    }
}