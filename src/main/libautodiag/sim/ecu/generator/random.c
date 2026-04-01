#include "libautodiag/sim/ecu/generator.h"

static Buffer * response_saej1979_dtcs(SimECUGenerator *generator, int service_id) {
    unsigned * seed = generator->context;
    return ad_buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1, seed);
}
static Buffer * response_saej1979_pid(SimECUGenerator *generator, final byte pid, int frameNumber) {
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
static Buffer * response_saej1979_vehicle_identification_request(SimECUGenerator * generator, Buffer * binRequest) {
    Buffer * payload = ad_buffer_new();
    int infoType_i = 1;
    byte infoType = binRequest->buffer[infoType_i];

    do {
        infoType = binRequest->buffer[infoType_i];
        switch(infoType) {
            case 0x00: {
                ad_buffer_append_byte(payload, infoType);
                ad_buffer_append_melt(payload, ad_buffer_from_ascii_hex("FFFFFFFF"));
            } break;
            default: {
                log_warn("multiple info type not implemented for info type %02X", infoType);
            } break;
        }
        infoType_i++;
    } while(generator->flavour.is_Iso15765_4 && infoType_i < binRequest->size);

    switch(infoType) {
        case 0x01: {
            if ( ! generator->flavour.is_Iso15765_4 ) {
                ad_buffer_append_byte(payload, infoType);
                // should not hardcode the vin message count here
                ad_buffer_append_byte(payload, 0x05);
            }
        } break;
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_VIN: {
            ad_buffer_append_byte(payload, infoType);
            if ( generator->flavour.is_Iso15765_4 ) {
                int number_of_data_items = 1;
                ad_buffer_append_byte(payload, number_of_data_items);                
            }
            ad_buffer_append_melt(payload,
                ad_buffer_new_random_with_seed(17, seed));
        } break;
        case 0x03: {
            if ( ! generator->flavour.is_Iso15765_4 ) {
                ad_buffer_append_byte(payload, infoType);
                // should not hardcode the message count here
                ad_buffer_append_byte(payload, 0x01);
            }
        } break;
        case 0x04: {
            ad_buffer_append_byte(payload, infoType);
            if ( generator->flavour.is_Iso15765_4 ) {
                int number_of_data_items = 1;
                ad_buffer_append_byte(payload, number_of_data_items);                
            }
            ad_buffer_append_melt(binResponse,
                ad_buffer_new_random_with_seed(16, seed));
        } break;
        case 0x05: {
            if ( ! generator->flavour.is_Iso15765_4 ) {
                ad_buffer_append_byte(payload, infoType);
                // should not hardcode the message count here
                ad_buffer_append_byte(payload, 0x01);
            }
        } break;
        case 0x06: {
            ad_buffer_append_byte(payload, infoType);
            if ( generator->flavour.is_Iso15765_4 ) {
                int number_of_data_items = 1;
                ad_buffer_append_byte(payload, number_of_data_items);                
            }
            ad_buffer_append_melt(payload,
                ad_buffer_new_random_with_seed(4, seed));
            break;
        }
        case 0x07: {
            if ( ! generator->flavour.is_Iso15765_4 ) {
                ad_buffer_append_byte(payload, infoType);
                // should not hardcode the message count here
                ad_buffer_append_byte(payload, 0x01);
            }
        } break;
        case 0x08: {
            ad_buffer_append_byte(payload, infoType);
            if ( generator->flavour.is_Iso15765_4 ) {
                int number_of_data_items = 1;
                ad_buffer_append_byte(payload, number_of_data_items);                
            }
            ad_buffer_append_melt(payload,
                ad_buffer_new_random_with_seed(4, seed));
        } break;
        case 0x09: {
            if ( ! generator->flavour.is_Iso15765_4 ) {
                ad_buffer_append_byte(payload, infoType);
                // should not hardcode the message count here
                ad_buffer_append_byte(payload, 0x01);
            }
        } break;
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: {
            final Buffer * name = ad_buffer_from_ascii("ECU random");
            ad_buffer_append_byte(payload, infoType);
            if ( generator->flavour.is_Iso15765_4 ) {
                int number_of_data_items = 1;
                ad_buffer_append_byte(payload, number_of_data_items);                
            }
            ad_buffer_padding(name, 20, 0x00);
            ad_buffer_append_melt(payload, name);
        } break;
    }

    Buffer * binResponse = ad_buffer_new();
    if ( generator->flavour.is_Iso15765_4 ) {
        ad_buffer_append_byte(binResponse, OBD_SERVICE_REQUEST_VEHICLE_INFORMATION | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE);
        ad_buffer_append(binResponse, payload);
    } else {
        // split message in multiple frames if not Iso15765-4
        if ( (infoType % 2) == 0 ) {
            for(int message_in = 1; message_in < payload->size; message_in+=4) {
                ad_buffer_append_byte(binResponse, OBD_SERVICE_REQUEST_VEHICLE_INFORMATION | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE);
                ad_buffer_append_byte(binResponse, infoType);
                ad_buffer_append_byte(binResponse, message_in);
                ad_buffer_slice_append(binResponse, payload, message_in, 4);
            }
        } else {
            ad_buffer_append_byte(binResponse, OBD_SERVICE_REQUEST_VEHICLE_INFORMATION | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE);
            ad_buffer_append_byte(binResponse, infoType);
            ad_buffer_append_melt(binResponse, payload);
        }
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
            return generator->response_saej1979_pids(generator, binRequest);

        case OBD_SERVICE_PENDING_DTC:
        case OBD_SERVICE_PERMANENT_DTC:
        case OBD_SERVICE_SHOW_DTC:
            return generator->response_saej1979_dtcs_wrapper(generator, binRequest->buffer[0]);

        case OBD_SERVICE_CLEAR_DTC: {
            log_msg(LOG_DEBUG, "Clearing DTCs");
        } break;

        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION:
            return response_saej1979_vehicle_identification_request(generator, binRequest);        
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
    generator->response_saej1979_pid = response_saej1979_pid;
    generator->response_saej1979_dtcs = response_saej1979_dtcs;
    unsigned * seed = (unsigned*)malloc(sizeof(unsigned));
    *seed = 1;
    generator->context = seed;
    return generator;
}
