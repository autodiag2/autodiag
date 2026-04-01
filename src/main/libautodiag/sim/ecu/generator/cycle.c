#include "libautodiag/sim/ecu/generator.h"

typedef struct {
    byte cycle_percent[256][256];
} GState;
static Buffer * response_saej1979_dtcs(SimECUGenerator *generator, int service_id) {
    GState * state = (GState*)generator->state;
    return ad_buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1,
                    state->cycle_percent[service_id][0]);
}
static Buffer * response_saej1979_pid(SimECUGenerator *generator, final byte pid, int frameNumber) {
    Buffer * binResponse = ad_buffer_new();
    GState * state = (GState*)generator->state;
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
                            ad_buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2,
                                state->cycle_percent[
                                    frameNumber == -1 ? OBD_SERVICE_SHOW_CURRENT_DATA : OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA
                                ][pid])
                        );
        } break;
    }
    return binResponse;
}
static Buffer * response_saej1979_vehicle_identification_request_info_type(SimECUGenerator * generator, byte infoType) {
    GState * state = (GState*)generator->state;
    switch(infoType) {
        case 0xC0:
        case 0xA0:
        case 0x80:
        case 0x60:
        case 0x40:
        case 0x20:
        case 0x00:                                          return ad_buffer_from_ascii_hex("FFFFFFFF");
        case 0x01:                                          return ad_buffer_from_ascii_hex("05");
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_VIN:   return ad_buffer_new_cycle(17,
                                                                state->cycle_percent[OBD_SERVICE_REQUEST_VEHICLE_INFORMATION][infoType]
                                                            );
        case 0x03:                                          return ad_buffer_from_ascii_hex("01");
        case 0x04:                                          return ad_buffer_new_cycle(16,
                                                                state->cycle_percent[OBD_SERVICE_REQUEST_VEHICLE_INFORMATION][infoType]
                                                            );
        case 0x05:                                          return ad_buffer_from_ascii_hex("01");
        case 0x06:                                          return ad_buffer_new_cycle(4,
                                                                state->cycle_percent[OBD_SERVICE_REQUEST_VEHICLE_INFORMATION][infoType]
                                                            );
        case 0x07:                                          return ad_buffer_from_ascii_hex("01");
        case 0x08:                                          return ad_buffer_new_cycle(4,
                                                                state->cycle_percent[OBD_SERVICE_REQUEST_VEHICLE_INFORMATION][infoType]
                                                            );
        case 0x09:                                          return ad_buffer_from_ascii_hex("01");
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: return ad_buffer_pad(ad_buffer_from_ascii("ECU cycle"), 20, 0x00);
    }
    return ad_buffer_new();
}
static Buffer * response(SimECUGenerator *generator, final Buffer *binRequest) {
    assert(generator->context != null);
    unsigned gears = *((unsigned*)generator->context);
    GState * state = (GState*)generator->state;
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

        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION: return generator->response_saej1979_vehicle_identification_request(generator, binRequest);
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
    generator->flavour.is_Iso15765_4 = 0;
    generator->response_saej1979_pid = response_saej1979_pid;
    generator->response_saej1979_dtcs = response_saej1979_dtcs;
    generator->response_saej1979_vehicle_identification_request_info_type = response_saej1979_vehicle_identification_request_info_type;
    GState * state = (GState*)calloc(1, sizeof(GState));
    generator->state = (void*)state;
    unsigned * gears = (unsigned*)malloc(sizeof(unsigned));
    *gears = 10;
    generator->context = gears;
    return generator;
}