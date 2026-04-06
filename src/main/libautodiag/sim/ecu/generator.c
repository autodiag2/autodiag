#include "libautodiag/sim/ecu/generator.h"

static Buffer * response_saej1979_dtcs_wrapper(SimECUGenerator *generator, int service_id) {
    Buffer * binResponse = ad_buffer_new();
    ad_buffer_append_byte(binResponse, service_id | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE);
    Buffer * payload = generator->response_saej1979_dtcs(generator, service_id);
    if ( generator->flavour.is_Iso15765_4 ) {
        ad_buffer_append_byte(binResponse, payload->size / 2);
    }
    ad_buffer_append_melt(binResponse,payload);
    return binResponse;
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

static Buffer * response_saej1979_vehicle_identification_request(SimECUGenerator * generator, Buffer * binRequest) {
    Buffer * payload = ad_buffer_new();
    int infoType_i = 1;
    byte infoType = binRequest->buffer[infoType_i];

    do {
        infoType = binRequest->buffer[infoType_i];
        switch(infoType) {
            case 0xC0:
            case 0xA0:
            case 0x80:
            case 0x60:
            case 0x40:
            case 0x20:
            case 0x00: {
                ad_buffer_append_byte(payload, infoType);
                ad_buffer_append_melt(payload, generator->response_saej1979_vehicle_identification_request_info_type(generator, infoType));
            } break;
            default: {
                log_warn("multiple info type not implemented for info type %02X", infoType);
            } break;
        }
        infoType_i++;
    } while(generator->flavour.is_Iso15765_4 && infoType_i < binRequest->size);

    switch(infoType) {
        case 0xC0:
        case 0xA0:
        case 0x80:
        case 0x60:
        case 0x40:
        case 0x20:
        case 0x00:
            break;
        default: {
            if ( infoType % 2 == 0 ) {
                ad_buffer_append_byte(payload, infoType);
                if ( generator->flavour.is_Iso15765_4 ) {
                    int number_of_data_items = 1;
                    ad_buffer_append_byte(payload, number_of_data_items);                
                }
                ad_buffer_append_melt(payload, generator->response_saej1979_vehicle_identification_request_info_type(generator, infoType));
            } else {
                if ( ! generator->flavour.is_Iso15765_4 ) {
                    ad_buffer_append_byte(payload, infoType);
                    ad_buffer_append_melt(payload, generator->response_saej1979_vehicle_identification_request_info_type(generator, infoType));
                }
            }
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
                ad_buffer_slice_append(binResponse, payload, message_in, min(4, payload->size - message_in));
            }
        } else {
            ad_buffer_append_byte(binResponse, OBD_SERVICE_REQUEST_VEHICLE_INFORMATION | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE);
            ad_buffer_append_byte(binResponse, infoType);
            ad_buffer_append_melt(binResponse, payload);
        }
    }
    return binResponse;
}
static Buffer * response_uds_did_wrapper(SimECUGenerator * generator, Buffer * binRequest) {
    Buffer * binResponse = ad_buffer_new();
    if (2 < binRequest->size) {
        if ((binRequest->size - 1) % 2 != 0) {
            sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
        } else {
            ad_buffer_append_byte(binResponse, binRequest->buffer[0] | UDS_POSITIVE_RESPONSE);
            for (unsigned i = 1; i < (unsigned)(binRequest->size - 1); i += 2) {
                final uint16_t did = (binRequest->buffer[i] << 8) | binRequest->buffer[i + 1];
                if ( generator->response_uds_did == null ) {
                    log_err("no handler attached");
                    break;
                }
                Buffer * dataResponse = generator->response_uds_did(generator, did);
                if ( 0 < dataResponse->size ) {
                    ad_buffer_append_uint16(binResponse, did);
                    ad_buffer_append_melt(binResponse, dataResponse);
                }
            }
        }
    } else {
        sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
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
    generator->response_saej1979_vehicle_identification_request = response_saej1979_vehicle_identification_request;
    generator->response_saej1979_vehicle_identification_request_info_type = null;
    generator->response_uds_did_wrapper = response_uds_did_wrapper;
    generator->response_uds_did = null;
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
        case AD_UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER: {
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