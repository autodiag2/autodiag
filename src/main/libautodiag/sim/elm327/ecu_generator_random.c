#include "libautodiag/sim/elm327/sim_generators.h"
#include "libautodiag/com/serial/elm/elm327/elm327.h"

void sim_ecu_generator_response_random(SimECUGenerator *generator, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin) {
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
    generator->sim_ecu_generator_response = SIM_ECU_GENERATOR_RESPONSE_FUNC(sim_ecu_generator_response_random);
    generator->type = strdup("random");
    return generator;
}