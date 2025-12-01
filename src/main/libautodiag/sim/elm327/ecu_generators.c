#include "libautodiag/sim/elm327/sim_generators.h"

SimECUGenerator * sim_ecu_generator_new() {
    SimECUGenerator * generator = (SimECUGenerator*)malloc(sizeof(SimECUGenerator));
    generator->context = null;
    generator->response = null;
    generator->response_for_python = null;
    generator->type = null;
    return generator;
}
void sim_ecu_generator_fill_nrc(Buffer * binResponse, final Buffer * binRequest, byte nrc) {
    assert(binResponse != null);
    assert(binRequest != null);
    assert(0 < binRequest->size);
    if ( 0 < binResponse->size ) {
        log_msg(LOG_DEBUG, "sim_ecu_generator_fill_nrc: binResponse is not empty (size=%d)", binResponse->size);
        buffer_recycle(binResponse);
    }
    buffer_append_byte(binResponse, OBD_DIAGNOSTIC_SERVICE_NEGATIVE_RESPONSE);
    buffer_append_byte(binResponse, binRequest->buffer[0]);
    buffer_append_byte(binResponse, nrc);
}
void sim_ecu_generator_fill_success(Buffer * binResponse, Buffer * binRequest) {
    assert(binResponse != null);
    assert(binRequest != null);
    assert(0 < binRequest->size);
    if ( 0 < binResponse->size ) {
        log_msg(LOG_DEBUG, "sim_ecu_generator_fill_success: binResponse is not empty (size=%d)", binResponse->size);
        buffer_recycle(binResponse);
    }
    buffer_append_byte(binResponse, binRequest->buffer[0] | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE);
    switch(binRequest->buffer[0]) {
        case OBD_SERVICE_SHOW_CURRENT_DATA:
        case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA:
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION:
            assert(1 < binRequest->size);
            buffer_append_byte(binResponse, binRequest->buffer[1]);
            break;
    }
}