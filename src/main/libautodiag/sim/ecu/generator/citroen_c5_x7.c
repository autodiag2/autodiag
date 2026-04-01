#include "libautodiag/sim/ecu/generator.h"

typedef struct {
    Buffer * vin;

    struct {
        int session_type;
        bool security_access_granted;
        ad_list_UDS_DTC * dtcs;
        byte DTCSupportedStatusMask;
        pthread_t * session_timer;
        bool session_continue;
    } uds;
    struct {
        ad_list_DTC * dtcs;
        bool mil_on;
    } obd;

} GState;

static void uds_reset_default_session(GState * state) {
    log_msg(LOG_DEBUG, "should reset controls and settings leaving in the ram");
    state->uds.security_access_granted = false;
    state->uds.session_type = UDS_SESSION_DEFAULT;
}
static void * session_timer_daemon(void *arg) {
    GState * state = (GState*)arg;
    while(state->uds.session_continue) {
        state->uds.session_continue = false;
        usleep(UDS_SESSION_TIMEOUT_MS * 1000);
    }
    uds_reset_default_session(state);
    free(state->uds.session_timer);
    state->uds.session_timer = null;
    return null;
}
static void start_or_update_session_timer(GState *state) {
    state->uds.session_continue = true;
    if ( state->uds.session_timer == null ) {
        state->uds.session_timer = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(state->uds.session_timer, null, session_timer_daemon, (void*)state);
    }
}
static bool service_is_uds(byte service) {
    return 0x10 <= service;
}
static bool uds_service_allowed(GState *state, byte service_id) {
    switch (service_id) {
        case UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL:
        case UDS_SERVICE_TESTER_PRESENT:
        case UDS_SERVICE_READ_DTC_INFORMATION:
        case UDS_SERVICE_READ_DATA_BY_IDENTIFIER:
            return true;
        
        case UDS_SERVICE_SECURITY_ACCESS:
            return state->uds.session_type == UDS_SESSION_PROGRAMMING;
        case UDS_SERVICE_ECU_RESET:
        case UDS_SERVICE_REQUEST_DOWNLOAD:
        case UDS_SERVICE_REQUEST_UPLOAD:
        case UDS_SERVICE_TRANSFER_DATA:
        case UDS_SERVICE_REQUEST_TRANSFER_EXIT:
        // unsure
        case UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER:
        case UDS_SERVICE_READ_MEMORY_BY_ADDRESS:
        case UDS_SERVICE_WRITE_MEMORY_BY_ADDRESS:
            return state->uds.session_type == UDS_SESSION_PROGRAMMING && state->uds.security_access_granted;

        case UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION:
            return state->uds.session_type == UDS_SESSION_EXTENDED_DIAGNOSTIC;

    }
    return false;
}
static Buffer * saej1979_response_dtcs(SimECUGenerator *generator, int service_id) {
    GState * state = (GState*)generator->state;
    Buffer * binResponse = ad_buffer_new();
    switch(service_id) {
        case OBD_SERVICE_SHOW_DTC: {
            for(int i = 0; i < state->obd.dtcs->size; i++) {
                final DTC * dtc = state->obd.dtcs->list[i];
                ad_buffer_append_byte(binResponse, dtc->data[0]);
                ad_buffer_append_byte(binResponse, dtc->data[1]);
            }
        } break;
        case OBD_SERVICE_PENDING_DTC: case OBD_SERVICE_PERMANENT_DTC: {
            ad_buffer_append_melt(binResponse, ad_buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1, seed));
        } break;
    }
    return binResponse;
}
static Buffer * saej1979_response_pid(SimECUGenerator *generator, final byte pid, int frameNumber) {
    unsigned * seed = generator->context;
    GState * state = (GState*)generator->state;
    Buffer * binResponse = ad_buffer_new();
    // Should append only bytes according to the PID, but for simplicity we just append random data
    switch(pid) {
        case 0x01: {
            Buffer* status = ad_buffer_new();
            ad_buffer_padding(status, 4, 0x00);
            status->buffer[0] = state->obd.dtcs->size;
            status->buffer[0] |= state->obd.mil_on << 7;
            ad_buffer_append_melt(binResponse, status);
        } break;
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
            ad_buffer_append_melt(binResponse,ad_buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, seed));
        } break;
    }
    return binResponse;
}
static Buffer * response(SimECUGenerator *generator, final Buffer *binRequest) {
    
    GState * state = (GState*)generator->state;
    final Buffer *binResponse = ad_buffer_new();
    if ( binRequest->size == 0 ) {
        return binResponse;
    }
    if ( ! sim_ecu_generator_fill_success(binResponse, binRequest) ) {
        return ad_buffer_new();
    }    
    start_or_update_session_timer(state);
    unsigned * seed = generator->context;

    if ( service_is_uds(binRequest->buffer[0]) ) {
        if ( ! uds_service_allowed(state, binRequest->buffer[0]) ) {
            sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_CONDITIONS_NOT_CORRECT);
            return binResponse;
        }
    }

    switch(binRequest->buffer[0]) {
        case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA:
        case OBD_SERVICE_SHOW_CURRENT_DATA:
            return generator->saej1979_response_pids(generator, binRequest);

        case OBD_SERVICE_PENDING_DTC:
        case OBD_SERVICE_PERMANENT_DTC:
        case OBD_SERVICE_SHOW_DTC:
            return generator->saej1979_response_dtcs_wrapper(generator, binRequest->buffer[0]);
        case OBD_SERVICE_CLEAR_DTC: {
            ad_list_DTC_clear(state->obd.dtcs);
            log_msg(LOG_DEBUG, "Clearing DTCs");
        } break;
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION: {
            if ( 1 < binRequest->size ) {            
                switch(binRequest->buffer[1]) {
                    case 0x00: {
                        ad_buffer_append_melt(binResponse, ad_buffer_from_ascii_hex("FFFFFFFFFF"));
                        break;
                    }
                    case 0x01: {
                        ad_buffer_append_byte(binResponse, 0x05);
                        break;
                    }
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_VIN: {
                        ad_buffer_append(binResponse,state->vin);                
                        break;
                    }
                    case 0x03: {
                        ad_buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x04: {
                        ad_buffer_append_melt(binResponse,ad_buffer_new_random_with_seed(16, seed));                
                        break;
                    }
                    case 0x05: {
                        ad_buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x06: {
                        ad_buffer_append_melt(binResponse,ad_buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x07: {
                        ad_buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case 0x08: {
                        ad_buffer_append_melt(binResponse,ad_buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x09: {
                        ad_buffer_append_byte(binResponse,0x01);
                        break;
                    }
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: {
                        final Buffer * name = ad_buffer_from_ascii("ECU citroen C5 X7");
                        ad_buffer_padding(name, 20, 0x00);
                        ad_buffer_append_melt(binResponse, name);
                        break;
                    }
                    case 0x0B: {
                        ad_buffer_append_melt(binResponse,ad_buffer_new_random_with_seed(4, seed));
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
                    uds_reset_default_session(state);
                } else {
                    state->uds.session_type = binRequest->buffer[1];
                }
                ad_buffer_append_byte(binResponse, state->uds.session_type);
                ad_buffer_append_melt(binResponse, ad_buffer_from_ints( 
                    0x00, 0x19, 0x07, 0xD0 
                ));
            }
        } break;
        case UDS_SERVICE_READ_DATA_BY_IDENTIFIER: {
            if ( 2 < binRequest->size ) {
                if ( (binRequest->size-1) % 2 != 0 ) {
                    sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                } else {
                    for(unsigned i = 1; i < (binRequest->size-1); i+=2) {
                        final int did = (binRequest->buffer[i] << 8) | binRequest->buffer[i+1];
                        ad_buffer_append_melt(binResponse, ad_buffer_from_ints(binRequest->buffer[i], binRequest->buffer[i+1]));
                        switch(did) {
                            case UDS_DID_Active_Diagnostic_Session_Data_Identifier_information: {
                                ad_buffer_append_byte(binResponse, state->uds.session_type);
                            } break;
                            case UDS_DID_VIN: {
                                ad_buffer_append(binResponse, state->vin);
                            } break;
                            default: {
                                ad_buffer_append(binResponse, ad_buffer_new_random_with_seed(10, seed));
                            } break;
                        }
                    }
                }
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case UDS_SERVICE_READ_DTC_INFORMATION: {
            if ( 1 < binRequest->size ) {
                ad_buffer_append_byte(binResponse, binRequest->buffer[1]);
                switch(binRequest->buffer[1]) {
                    case UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_FIRST_CONFIRMED_DTC: {
                        ad_buffer_append_byte(binResponse, state->uds.DTCSupportedStatusMask);
                        for(int i = 0; i < state->uds.dtcs->size; i++) {
                            final UDS_DTC * dtc = state->uds.dtcs->list[i];
                            ad_buffer_append_bytes(binResponse, dtc->data, DTC_DATA_SZ);
                            ad_buffer_append_byte(binResponse, dtc->status);
                        }
                    } break;
                    case UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_DTC_BY_SEVERITY_MASK_RECORD: {
                        if ( binRequest->size <= 3 ) {
                            sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                        } else {
                            final byte DTCSeverityMask = binRequest->buffer[2];
                            final byte DTCStatusMask = binRequest->buffer[3];
                            ad_buffer_append_byte(binResponse, state->uds.DTCSupportedStatusMask);
                            for(int i = 0; i < state->uds.dtcs->size; i++) {
                                final UDS_DTC * dtc = state->uds.dtcs->list[i];
                                ad_buffer_append_byte(binResponse, 0xFF); // DTCSeverity
                                ad_buffer_append_byte(binResponse, 0xFF); // DTCFunctionalUnit
                                ad_buffer_append_bytes(binResponse, dtc->data, DTC_DATA_SZ);
                                ad_buffer_append_byte(binResponse, dtc->status);
                            }
                        }
                    } break;
                    case UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_DTC_BY_STATUS_MASK: {
                        if ( binRequest->size <= 2 ) {
                            sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                        } else {
                            final byte DTCStatusMask = binRequest->buffer[2];
                            ad_buffer_append_byte(binResponse, state->uds.DTCSupportedStatusMask);
                            for(int i = 0; i < state->uds.dtcs->size; i++) {
                                final UDS_DTC * dtc = state->uds.dtcs->list[i];
                                if ( (dtc->status & DTCStatusMask) != 0 ) {
                                    ad_buffer_append_bytes(binResponse, dtc->data, DTC_DATA_SZ);
                                    ad_buffer_append_byte(binResponse, dtc->status);
                                }
                            }
                        }
                    } break;
                }
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case UDS_SERVICE_TESTER_PRESENT: {
            if ( 1 < binRequest->size ) {
                switch(binRequest->buffer[1]) {
                    case UDS_TESTER_PRESENT_SUB_ZERO:
                        ad_buffer_append_byte(binResponse, binRequest->buffer[1]);
                        break;
                    case UDS_TESTER_PRESENT_SUB_NO_RESPONSE:
                        ad_buffer_append_byte(binResponse, binRequest->buffer[1]);
                        log_msg(LOG_DEBUG, "TODO (not sending anything back)");
                        break;
                    default:
                        log_msg(LOG_INFO, "unsupported sub function %02hhX", binRequest->buffer[1]);
                        sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
                        break;
                }
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION: {
            if ( 3 < binRequest->size ) {
                ad_list_DTC_clear((ad_list_DTC*)state->uds.dtcs);
                ad_list_DTC_clear(state->obd.dtcs);
                log_msg(LOG_DEBUG, "Should apply the group mask from the request");
                ad_buffer_slice_append(binResponse, binRequest, 1, 3);
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case UDS_SERVICE_SECURITY_ACCESS: {
            if ( 1 < binRequest->size ) {
                final int securit_seed = 0x4321;
                switch(binRequest->buffer[1]) {
                    case UDS_SECURITY_ACCESS_ECU_GENERATOR_CITROEN_C5_X7_SEED: {
                        ad_buffer_append_byte(binResponse, binRequest->buffer[1]);
                        ad_buffer_append_byte(binResponse, securit_seed >> 8);
                        ad_buffer_append_byte(binResponse, securit_seed & 0xFF);
                    } break;
                    case UDS_SECURITY_ACCESS_ECU_GENERATOR_CITROEN_C5_X7_KEY: {
                        assert(3 < binRequest->size);
                        int encrypted = binRequest->buffer[2] << 8 | binRequest->buffer[3];
                        int decrypted = uds_security_access_ecu_generator_citroen_c5_x7_encrypt(encrypted);
                        log_msg(LOG_DEBUG, "From emu: encrypted received: 0x%X decrypted to 0x%X", encrypted, decrypted);
                        if ( securit_seed == decrypted ) {
                            ad_buffer_append_byte(binResponse, binRequest->buffer[1]);
                            state->uds.security_access_granted = true;
                        } else {
                            sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_SECURITY_ACCESS_DENIED);
                        }
                    } break;
                    default: {
                        sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
                    } break;
                }
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case UDS_SERVICE_ECU_RESET: {
            if ( 1 < binRequest->size ) {
                final byte resetType = binRequest->buffer[1];
                uds_reset_default_session(state);
                ad_buffer_append_byte(binResponse, resetType);
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER: {
            if ( 2 < binRequest->size ) {
                Buffer * did_buffer = ad_buffer_slice(binRequest, 1, 2);
                uint16_t did = ad_buffer_to_be16(did_buffer);
                ad_buffer_free(did_buffer);
                switch(did) {
                    case UDS_DID_VIN: {
                        if ( (3 + 17) == binRequest->size ) {
                            state->vin = ad_buffer_slice(binRequest, 3, 17);
                        } else {
                            sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_REQUEST_OUT_OF_RANGE);
                        }
                    } break;
                }
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
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
SimECUGenerator* sim_ecu_generator_new_citroen_c5_x7() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->response = SIM_ECU_GENERATOR_RESPONSE(response);
    generator->context_load_from_string = SIM_ECU_GENERATOR_CONTEXT_LOAD_FROM_STRING(context_load_from_string);
    generator->context_to_string = SIM_ECU_GENERATOR_CONTEXT_TO_STRING(context_to_string);
    generator->type = strdup("Citroen C5 X7");
    generator->flavour.is_Iso15765_4 = false;
    generator->saej1979_response_pid = saej1979_response_pid;
    generator->saej1979_response_dtcs = saej1979_response_dtcs;
    generator->state = (GState*)malloc(sizeof(GState));
    GState * state = (GState*)generator->state;
    state->vin = null;
    state->obd.dtcs = null;
    state->obd.mil_on = true;
    state->uds.session_type = UDS_SESSION_DEFAULT;
    state->uds.security_access_granted = false;
    state->uds.dtcs = null;
    state->uds.DTCSupportedStatusMask = 
            UDS_DTC_STATUS_TestFailed | UDS_DTC_STATUS_TestFailedThisOperationCycle |
            UDS_DTC_STATUS_PendingDTC | UDS_DTC_STATUS_ConfirmedDTC |
            UDS_DTC_STATUS_TestNotCompletedSinceLastClear | UDS_DTC_STATUS_TestFailedSinceLastClear |
            UDS_DTC_STATUS_TestNotCompletedThisOperationCycle | UDS_DTC_STATUS_WarningIndicatorRequested;
    state->uds.session_timer = null;
    state->uds.session_continue = true;

    state->vin = ad_buffer_from_ascii("VF1BB05CF26010203");
    state->uds.dtcs = ad_list_UDS_DTC_new();
    state->obd.dtcs = ad_list_DTC_new();
    generator->context = (unsigned*)malloc(sizeof(unsigned));
    *((unsigned *)generator->context) = 1;

    ad_list_ad_object_string * dtcs = ad_list_ad_object_string_new();
    ad_list_ad_object_string_append(dtcs, ad_object_string_new_from("P0103"));
    ad_list_ad_object_string_append(dtcs, ad_object_string_new_from("P0104"));
    for(int i = 0; i < dtcs->size; i++) {
        SAEJ1979_DTC * dtc = saej1979_dtc_from_string(dtcs->list[i]->data);
        ad_list_DTC_append(state->obd.dtcs, (DTC*)dtc);
        ad_list_UDS_DTC_append(state->uds.dtcs, UDS_DTC_new_from(dtc));
    }
    return generator;
}