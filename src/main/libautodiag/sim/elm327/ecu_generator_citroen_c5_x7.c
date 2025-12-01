#include "libautodiag/sim/elm327/sim_generators.h"
#include "libautodiag/com/serial/elm/elm327/elm327.h"
#include "libautodiag/com/uds/uds.h"

typedef struct {
    Buffer * vin;

    struct {
        int session_type;
        bool security_access_granted;
        list_UDS_DTC * dtcs;
        byte DTCSupportedStatusMask;
        pthread_t session_timer;
        bool session_continue;
    } uds;
    struct {
        list_DTC * dtcs;
        bool mil_on;
    } obd;

} VehicleState;

static VehicleState state = {
    .vin = null,
    .uds = {
        .session_type = UDS_SESSION_DEFAULT,
        .security_access_granted = false,
        .dtcs = null,
        .DTCSupportedStatusMask = 
            UDS_DTC_STATUS_TestFailed | UDS_DTC_STATUS_TestFailedThisOperationCycle |
            UDS_DTC_STATUS_PendingDTC | UDS_DTC_STATUS_ConfirmedDTC |
            UDS_DTC_STATUS_TestNotCompletedSinceLastClear | UDS_DTC_STATUS_TestFailedSinceLastClear |
            UDS_DTC_STATUS_TestNotCompletedThisOperationCycle | UDS_DTC_STATUS_WarningIndicatorRequested,
        .session_timer = null,
        .session_continue = true
    },
    .obd = {
        .dtcs = null,
        .mil_on = true
    }
};
static void uds_reset_default_session() {
    log_msg(LOG_DEBUG, "should reset controls and settings leaving in the ram");
    state.uds.security_access_granted = false;
    state.uds.session_type = UDS_SESSION_DEFAULT;
}
static void * session_timer_daemon(void *arg) {
    while(state.uds.session_continue) {
        state.uds.session_continue = false;
        usleep(UDS_SESSION_TIMEOUT_MS * 1000);
    }
    uds_reset_default_session();
    free(state.uds.session_timer);
    state.uds.session_timer = null;
}
static void start_or_update_session_timer() {
    state.uds.session_continue = true;
    if ( state.uds.session_timer == null ) {
        state.uds.session_timer = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(state.uds.session_timer, null, session_timer_daemon, null);
    }
}
static bool service_is_uds(byte service) {
    return 0x10 <= service;
}
static bool uds_service_allowed(byte service_id) {
    switch (service_id) {
        case UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL:
        case UDS_SERVICE_TESTER_PRESENT:
        case UDS_SERVICE_READ_DTC_INFORMATION:
        case UDS_SERVICE_READ_DATA_BY_IDENTIFIER:
            return true;
        
        case UDS_SERVICE_SECURITY_ACCESS:
            return state.uds.session_type == UDS_SESSION_PROGRAMMING;
        case UDS_SERVICE_ECU_RESET:
        case UDS_SERVICE_REQUEST_DOWNLOAD:
        case UDS_SERVICE_REQUEST_UPLOAD:
        case UDS_SERVICE_TRANSFER_DATA:
        case UDS_SERVICE_REQUEST_TRANSFER_EXIT:
        // unsure
        case UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER:
        case UDS_SERVICE_READ_MEMORY_BY_ADDRESS:
        case UDS_SERVICE_WRITE_MEMORY_BY_ADDRESS:
            return state.uds.session_type == UDS_SESSION_PROGRAMMING && state.uds.security_access_granted;

        case UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION:
            return state.uds.session_type == UDS_SESSION_EXTENDED_DIAGNOSTIC;

    }
    return false;
}

static bool response(SimECUGenerator *generator, final Buffer *binResponse, final Buffer *binRequest) {
    
    start_or_update_session_timer();

    bool responseStatus = true;
    unsigned * seed = generator->context;
    if ( seed == null ) {
        seed = (unsigned*)malloc(sizeof(unsigned));
        *seed = 1;
        generator->context = seed;
    }

    if ( service_is_uds(binRequest->buffer[0]) ) {
        if ( ! uds_service_allowed(binRequest->buffer[0]) ) {
            buffer_append_byte(binResponse, UDS_NRC_CONDITIONS_NOT_CORRECT);
            return false;
        }
    }

    switch(binRequest->buffer[0]) {
        case OBD_SERVICE_SHOW_CURRENT_DATA: {
            bool generic_behaviour = true;
            switch(binRequest->buffer[1]) {
                case 0x01: {
                    Buffer* status = buffer_new();
                    buffer_padding(status, 4, 0x00);
                    status->buffer[0] = state.obd.dtcs->size;
                    status->buffer[0] |= state.obd.mil_on << 7;
                    buffer_append(binResponse, status);
                    generic_behaviour = false;
                }
            }
            if ( generic_behaviour ) {
                buffer_append(binResponse,buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, seed));
            }
        } break;
        case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA: {
            buffer_append(binResponse,buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, seed));
        } break;
        case OBD_SERVICE_SHOW_DTC: {
            for(int i = 0; i < state.obd.dtcs->size; i++) {
                final DTC * dtc = state.obd.dtcs->list[i];
                buffer_append_byte(binResponse, dtc->data[0]);
                buffer_append_byte(binResponse, dtc->data[1]);
            }
        } break;
        case OBD_SERVICE_PENDING_DTC: case OBD_SERVICE_PERMANENT_DTC: {
            buffer_append(binResponse,buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1, seed));                
        } break;
        case OBD_SERVICE_CLEAR_DTC: {
            list_DTC_clear(state.obd.dtcs);
            buffer_append_byte(binResponse, 0xAA); // Random byte so the response is not empty
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
                        buffer_append(binResponse,state.vin);                
                        break;
                    }
                    case 0x03: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x04: {
                        buffer_append(binResponse,buffer_new_random_with_seed(16, seed));                
                        break;
                    }
                    case 0x05: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x06: {
                        buffer_append(binResponse,buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x07: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x08: {
                        buffer_append(binResponse,buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x09: {
                        buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: {
                        final Buffer * name = buffer_from_ascii("ECU citroen C5 X7");
                        buffer_padding(name, 20, 0x00);
                        buffer_append(binResponse, name);
                        break;
                    }
                    case 0x0B: {
                        buffer_append(binResponse,buffer_new_random_with_seed(4, seed));
                        break;
                    }
                }
            } else {
                log_msg(LOG_ERROR, "Missing PID");
            }
        } break;
        case UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL: {
            if ( 1 < binRequest->size ) {
                if ( binRequest->buffer[1] == UDS_SESSION_DEFAULT ) {
                    uds_reset_default_session();
                } else {
                    state.uds.session_type = binRequest->buffer[1];
                }
                buffer_append_byte(binResponse, state.uds.session_type);
                buffer_append(binResponse, buffer_from_ints( 
                    0x00, 0x19, 0x07, 0xD0 
                ));
            }
        } break;
        case UDS_SERVICE_READ_DATA_BY_IDENTIFIER: {
            if ( 2 < binRequest->size ) {
                if ( (binRequest->size-1) % 2 != 0 ) {
                    responseStatus = false;
                    buffer_append_byte(binResponse, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                } else {
                    for(int i = 1; i < (binRequest->size-1); i+=2) {
                        final int did = (binRequest->buffer[i] << 8) | binRequest->buffer[i+1];
                        buffer_append(binResponse, buffer_from_ints(binRequest->buffer[i], binRequest->buffer[i+1]));
                        switch(did) {
                            case UDS_DID_Active_Diagnostic_Session_Data_Identifier_information: {
                                buffer_append_byte(binResponse, state.uds.session_type);
                            } break;
                            case UDS_DID_VIN: {
                                buffer_append(binResponse, state.vin);
                            } break;
                        }
                    }
                }
            } else {
                responseStatus = false;
                buffer_append_byte(binResponse, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case UDS_SERVICE_READ_DTC_INFORMATION: {
            if ( 1 < binRequest->size ) {
                buffer_append_byte(binResponse, binRequest->buffer[1]);
                switch(binRequest->buffer[1]) {
                    case UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_FIRST_CONFIRMED_DTC: {
                        buffer_append_byte(binResponse, state.uds.DTCSupportedStatusMask);
                        for(int i = 0; i < state.uds.dtcs->size; i++) {
                            final UDS_DTC * dtc = state.uds.dtcs->list[i];
                            buffer_append_bytes(binResponse, dtc->data, DTC_DATA_SZ);
                            buffer_append_byte(binResponse, dtc->status);
                        }
                    } break;
                    case UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_DTC_BY_SEVERITY_MASK_RECORD: {
                        if ( binRequest->size <= 3 ) {
                            responseStatus = false;
                            buffer_append_byte(binResponse, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                        } else {
                            final byte DTCSeverityMask = binRequest->buffer[2];
                            final byte DTCStatusMask = binRequest->buffer[3];
                            buffer_append_byte(binResponse, state.uds.DTCSupportedStatusMask);
                            for(int i = 0; i < state.uds.dtcs->size; i++) {
                                final UDS_DTC * dtc = state.uds.dtcs->list[i];
                                buffer_append_byte(binResponse, 0xFF); // DTCSeverity
                                buffer_append_byte(binResponse, 0xFF); // DTCFunctionalUnit
                                buffer_append_bytes(binResponse, dtc->data, DTC_DATA_SZ);
                                buffer_append_byte(binResponse, dtc->status);
                            }
                        }
                    } break;
                    case UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_DTC_BY_STATUS_MASK: {
                        if ( binRequest->size <= 2 ) {
                            responseStatus = false;
                            buffer_append_byte(binResponse, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                        } else {
                            final byte DTCStatusMask = binRequest->buffer[2];
                            buffer_append_byte(binResponse, state.uds.DTCSupportedStatusMask);
                            for(int i = 0; i < state.uds.dtcs->size; i++) {
                                final UDS_DTC * dtc = state.uds.dtcs->list[i];
                                if ( (dtc->status & DTCStatusMask) != 0 ) {
                                    buffer_append_bytes(binResponse, dtc->data, DTC_DATA_SZ);
                                    buffer_append_byte(binResponse, dtc->status);
                                }
                            }
                        }
                    } break;
                }
            } else {
                responseStatus = false;
                buffer_append_byte(binResponse, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case UDS_SERVICE_TESTER_PRESENT: {
            if ( 1 < binRequest->size ) {
                switch(binRequest->buffer[1]) {
                    case UDS_TESTER_PRESENT_SUB_ZERO:
                        buffer_append_byte(binResponse, binRequest->buffer[1]);
                        break;
                    case UDS_TESTER_PRESENT_SUB_NO_RESPONSE:
                        buffer_append_byte(binResponse, binRequest->buffer[1]);
                        log_msg(LOG_DEBUG, "TODO (not sending anything back)");
                        break;
                    default:
                        log_msg(LOG_INFO, "unsupported sub function %02hhX", binRequest->buffer[1]);
                        responseStatus = false;
                        buffer_append_byte(binResponse, UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
                        break;
                }
            } else {
                responseStatus = false;
                buffer_append_byte(binResponse, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION: {
            if ( 3 < binRequest->size ) {
                list_DTC_clear(state.uds.dtcs);
                list_DTC_clear(state.obd.dtcs);
                log_msg(LOG_DEBUG, "Should apply the group mask from the request");
                buffer_slice_append(binResponse, binRequest, 1, 3);
            } else {
                responseStatus = false;
                buffer_append_byte(binResponse, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case UDS_SERVICE_SECURITY_ACCESS: {
            if ( 1 < binRequest->size ) {
                final int seed = 0x4321;
                switch(binRequest->buffer[1]) {
                    case UDS_SECURITY_ACCESS_ECU_GENERATOR_CITROEN_C5_X7_SEED: {
                        buffer_append_byte(binResponse, binRequest->buffer[1]);
                        buffer_append_byte(binResponse, seed >> 8);
                        buffer_append_byte(binResponse, seed & 0xFF);
                    } break;
                    case UDS_SECURITY_ACCESS_ECU_GENERATOR_CITROEN_C5_X7_KEY: {
                        assert(3 < binRequest->size);
                        int encrypted = binRequest->buffer[2] << 8 | binRequest->buffer[3];
                        int decrypted = uds_security_access_ecu_generator_citroen_c5_x7_encrypt(encrypted);
                        log_msg(LOG_DEBUG, "From emu: encrypted received: 0x%X decrypted to 0x%X", encrypted, decrypted);
                        if ( seed == decrypted ) {
                            buffer_append_byte(binResponse, binRequest->buffer[1]);
                            state.uds.security_access_granted = true;
                        } else {
                            responseStatus = false;
                            buffer_append_byte(binResponse, UDS_NRC_SECURITY_ACCESS_DENIED);
                        }
                    } break;
                    default: {
                        responseStatus = false;
                        buffer_append_byte(binResponse, UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
                    } break;
                }
            } else {
                responseStatus = false;
                buffer_append_byte(binResponse, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
    }
    return responseStatus;
}
SimECUGenerator* sim_ecu_generator_new_citroen_c5_x7() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->response = SIM_ECU_GENERATOR_RESPONSE_FUNC(response);
    generator->type = strdup("Citroen C5 X7");
    state.vin = buffer_from_ascii("VF7RD5FV8FL507366");
    state.uds.dtcs = list_UDS_DTC_new();
    state.obd.dtcs = list_DTC_new();

    list_object_string * dtcs = list_object_string_new();
    list_object_string_append(dtcs, object_string_new_from("P0103"));
    list_object_string_append(dtcs, object_string_new_from("P0104"));
    for(int i = 0; i < dtcs->size; i++) {
        SAEJ1979_DTC * dtc = saej1979_dtc_from_string(dtcs->list[i]->data);
        list_DTC_append(state.obd.dtcs, dtc);
        list_Buffer_append(state.uds.dtcs, UDS_DTC_new_from(dtc));
    }
    return generator;
}