#include "libautodiag/sim/ecu/generator.h"

static Buffer * response_saej1979_dtcs_wrapper(SimECUGenerator *generator, int service_id) {
    Buffer * binResponse = ad_buffer_new();
    ad_buffer_append_byte(binResponse, service_id | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE);
    Buffer * payload = generator->response_saej1979_dtcs(generator, service_id);
    if ( generator->flavour.is_Iso15765_4 ) {
        ad_buffer_append_byte(binResponse, payload->size / 2);
    }
    ad_buffer_append_melt(binResponse,payload);
    return binResponse
}
static Buffer * response_saej1979_pids(SimECUGenerator *generator, final Buffer *binRequest) {
    if ( 0 == binRequest->size ) {
        return ad_buffer_new();
    }
    bool pidHasFrameNumber = false;
    Buffer * binResponse = ad_buffer_new();
    switch(binRequest->buffer[0]) {
        case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA:
            pidHasFrameNumber = true;
        case OBD_SERVICE_SHOW_CURRENT_DATA: {
            int pid_i = 1;
            ad_buffer_append_byte(binResponse, binRequest->buffer[0] | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE);
            if ( binRequest->size <= pid_i ) {
                ad_buffer_recycle(binResponse);
                break;
            }
            if ( generator->response_saej1979_pid == null ) {
                log_err("response_saej1979_pid is not set for generator of type %s", generator->type);
                ad_buffer_recycle(binResponse);
                break;
            }
            do {
                ad_buffer_append_byte(binResponse, binRequest->buffer[pid_i]);
                int frameNumber = -1;
                if ( pidHasFrameNumber ) {
                    frameNumber = binRequest->buffer[pid_i+1];
                    ad_buffer_append_byte(binResponse, frameNumber);
                }
                ad_buffer_append_melt(
                    binResponse,
                    generator->response_saej1979_pid(generator, binRequest->buffer[pid_i], frameNumber)
                );
                pid_i += 1 + pidHasFrameNumber;
            } while(generator->flavour.is_Iso15765_4 && pid_i < binRequest->size);
        } break;
    }
    return binResponse;
}
SimECUGenerator * sim_ecu_generator_new() {
    SimECUGenerator * generator = (SimECUGenerator*)malloc(sizeof(SimECUGenerator));
    generator->context = null;
    generator->response = null;
    generator->response_for_python = null;
    generator->response_saej1979_pids = response_saej1979_pids;
    generator->response_saej1979_pid = null;
    generator->response_saej1979_dtcs_wrapper = response_saej1979_dtcs_wrapper;
    generator->response_saej1979_dtcs = null;
    generator->type = null;
    generator->state = null;
    return generator;
}
void sim_ecu_generator_fill_nrc(Buffer * binResponse, final Buffer * binRequest, byte nrc) {
    assert(binResponse != null);
    assert(binRequest != null);
    assert(0 < binRequest->size);
    if ( 0 < binResponse->size ) {
        log_msg(LOG_DEBUG, "sim_ecu_generator_fill_nrc: binResponse is not empty (size=%d)", binResponse->size);
        ad_buffer_recycle(binResponse);
    }
    ad_buffer_append_byte(binResponse, OBD_DIAGNOSTIC_SERVICE_NEGATIVE_RESPONSE);
    ad_buffer_append_byte(binResponse, binRequest->buffer[0]);
    ad_buffer_append_byte(binResponse, nrc);
}
bool sim_ecu_generator_fill_success(Buffer * binResponse, Buffer * binRequest) {
    assert(binResponse != null);
    assert(binRequest != null);
    assert(0 < binRequest->size);
    if ( 0 < binResponse->size ) {
        log_msg(LOG_DEBUG, "sim_ecu_generator_fill_success: binResponse is not empty (size=%d)", binResponse->size);
        ad_buffer_recycle(binResponse);
    }
    ad_buffer_append_byte(binResponse, binRequest->buffer[0] | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE);
    switch(binRequest->buffer[0]) {
        case OBD_SERVICE_SHOW_CURRENT_DATA:
        case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA:
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION: {
            if ( 1 < binRequest->size ) {
                ad_buffer_append_byte(binResponse, binRequest->buffer[1]);
            } else {
                ad_buffer_recycle(binResponse);
                return false;
            }
        } break;
        // Only one data to write per request
        case UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER: {
            if ( 2 < binRequest->size ) {
                AD_BUFFER_APPEND_BYTES(binResponse, binRequest->buffer[1], binRequest->buffer[2]);
            } else {
                log_debug("Should send incorrect message length");
                ad_buffer_recycle(binResponse);
                return false;
            }
        } break;
    }
    return true;
}