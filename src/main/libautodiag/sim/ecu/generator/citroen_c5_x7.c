#include "libautodiag/sim/ecu/generator.h"

typedef struct {
    Buffer * vin;

    struct {
        bool security_access_granted;
        ad_list_AD_UDS_DTC * dtcs;
        byte DTCSupportedStatusMask;
        pthread_t * session_timer;
        bool session_continue;
        Buffer * did[0xFFFF];
        Buffer * memory;
    } uds;
    struct {
        ad_list_DTC * dtcs;
        bool mil_on;
    } obd;

} GState;

#define GSTATE_UDS_MEMORY_SZ 0xFFFF

static int get_session_type(GState *state) {
    Buffer * b = state->uds.did[AD_UDS_DID_Active_Diagnostic_Session_Data_Identifier_information];
    if ( b->size == 0 ) {
        return -1;
    }
    return b->buffer[0];
}
static void set_session_type(GState * state, byte session_type) {
    state->uds.did[AD_UDS_DID_Active_Diagnostic_Session_Data_Identifier_information] = ad_buffer_be_from_uint8(session_type);
}
static void ad_uds_reset_default_session(GState * state) {
    log_msg(LOG_DEBUG, "should reset controls and settings leaving in the ram");
    state->uds.security_access_granted = false;
    set_session_type(state, AD_UDS_SESSION_DEFAULT);
    state->uds.did[AD_UDS_DID_Active_Diagnostic_Session_Data_Identifier_information] = ad_buffer_be_from_uint8(AD_UDS_SESSION_DEFAULT);
}
static void * session_timer_daemon(void *arg) {
    GState * state = (GState*)arg;
    while(state->uds.session_continue) {
        state->uds.session_continue = false;
        usleep(AD_UDS_SESSION_TIMEOUT_MS * 1000);
    }
    ad_uds_reset_default_session(state);
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
static bool ad_uds_service_allowed(GState *state, byte service_id) {
    switch (service_id) {
        case AD_UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL:
        case AD_UDS_SERVICE_TESTER_PRESENT:
        case AD_UDS_SERVICE_READ_DTC_INFORMATION:
        case AD_UDS_SERVICE_READ_DATA_BY_IDENTIFIER:
            return true;
        
        case AD_UDS_SERVICE_SECURITY_ACCESS:
            return get_session_type(state) == AD_UDS_SESSION_PROGRAMMING;
        case AD_UDS_SERVICE_ECU_RESET:
        case AD_UDS_SERVICE_REQUEST_DOWNLOAD:
        case AD_UDS_SERVICE_REQUEST_UPLOAD:
        case AD_UDS_SERVICE_TRANSFER_DATA:
        case AD_UDS_SERVICE_REQUEST_TRANSFER_EXIT:
        // unsure
        case AD_UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER:
        case AD_UDS_SERVICE_READ_MEMORY_BY_ADDRESS:
        case AD_UDS_SERVICE_WRITE_MEMORY_BY_ADDRESS:
            return get_session_type(state) == AD_UDS_SESSION_PROGRAMMING && state->uds.security_access_granted;

        case AD_UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION:
            return get_session_type(state) == AD_UDS_SESSION_EXTENDED_DIAGNOSTIC;

    }
    return false;
}
static Buffer * response_saej1979_dtcs(SimECUGenerator *generator, int service_id) {
    GState * state = (GState*)generator->state;
    unsigned * seed = generator->context;
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
static Buffer * response_saej1979_pid(SimECUGenerator *generator, final byte pid, int frameNumber) {
    unsigned * seed = generator->context;
    GState * state = (GState*)generator->state;
    Buffer * binResponse = ad_buffer_new();
    // Should append only bytes according to the PID, but for simplicity we just append random data
    switch(pid) {
        case 0x01: {
            Buffer* status = ad_buffer_new();
            ad_buffer_pad(status, 4, 0x00);
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
            ad_buffer_append_melt(binResponse, ad_buffer_from_ascii_hex("FFFFFFFF"));
        } break;
        default: {
            ad_buffer_append_melt(binResponse,ad_buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, seed));
        } break;
    }
    return binResponse;
}
static Buffer * response_saej1979_vehicle_identification_request_info_type(SimECUGenerator * generator, byte infoType) {
    GState * state = (GState*)generator->state;
    unsigned * seed = generator->context;
    switch(infoType) {
        case 0x00:                                          return ad_buffer_from_ascii_hex("FFFFFFFF");
        case 0x01:                                          return ad_buffer_from_ascii_hex("05");
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_VIN:   return ad_buffer_copy(state->vin);
        case 0x03:                                          return ad_buffer_from_ascii_hex("01");
        case 0x04:                                          return ad_buffer_new_random_with_seed(16, seed);
        case 0x05:                                          return ad_buffer_from_ascii_hex("01");
        case 0x06:                                          return ad_buffer_new_random_with_seed(4, seed);
        case 0x07:                                          return ad_buffer_from_ascii_hex("01");
        case 0x08:                                          return ad_buffer_new_random_with_seed(4, seed);
        case 0x09:                                          return ad_buffer_from_ascii_hex("01");
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: return ad_buffer_pad(ad_buffer_from_ascii("ECU citroen C5 X7"), 20, 0x00);
    }
    return ad_buffer_new();
}
static void init_did(GState *state, unsigned *seed) {
    int i;

    if (state == null) {
        return;
    }

    for (i = 0; i < 0xFFFF; i++) {
        state->uds.did[i] = null;
    }

    state->uds.did[AD_UDS_DID_bootSoftwareIdentificationDataIdentifier] = ad_buffer_from_ascii("C5X7_BTL_ECU_9666912580");
    state->uds.did[AD_UDS_DID_applicationSoftwareIdentificationDataIdentifier] = ad_buffer_from_ascii("C5X7_APP_ECU_9666912580");
    state->uds.did[AD_UDS_DID_applicationDataIdentification] = ad_buffer_from_ascii("C5X7_CAL_2.0HDI_163");
    state->uds.did[AD_UDS_DID_bootSoftwareFingerprint] = ad_buffer_from_ascii("BTLFP20100415");
    state->uds.did[AD_UDS_DID_applicationSoftwareFingerprint] = ad_buffer_from_ascii("APPFP20110422");
    state->uds.did[AD_UDS_DID_applicationDataFingerprint] = ad_buffer_from_ascii("CALFP20110503");
    state->uds.did[AD_UDS_DID_Active_Diagnostic_Session_Data_Identifier_information] = ad_buffer_be_from_uint8(AD_UDS_SESSION_DEFAULT);
    state->uds.did[AD_UDS_DID_manufacturerSparePartNumber] = ad_buffer_from_ascii("9666912580");
    state->uds.did[AD_UDS_DID_manufacturerECUSoftwareNumber] = ad_buffer_from_ascii("9675495080");
    state->uds.did[AD_UDS_DID_manufacturerECUSoftwareVersion] = ad_buffer_from_ascii("SW16.1");
    state->uds.did[AD_UDS_DID_identifierOfSystemSupplier] = ad_buffer_from_ascii("CONTINENTAL");
    state->uds.did[AD_UDS_DID_ECUManufacturingDate] = ad_buffer_from_ascii("20110321");
    state->uds.did[AD_UDS_DID_ECUSerialNumber] = ad_buffer_from_ascii("C5X7ECU00000001");
    state->uds.did[AD_UDS_DID_SupportedFunctionnalUnit] = ad_buffer_from_ints(0x00, 0x01);
    state->uds.did[AD_UDS_DID_ManufacturerKitAssemblyPartNumber] = ad_buffer_from_ascii("KITC5X7ECM001");

    if (state->vin != null && state->vin->size == 17) {
        state->uds.did[AD_UDS_DID_VIN] = ad_buffer_copy(state->vin);
    } else {
        state->uds.did[AD_UDS_DID_VIN] = ad_buffer_from_ascii("VF7RD4HTHBL123456");
    }

    state->uds.did[AD_UDS_DID_system_supplier_ECU_hardware_number] = ad_buffer_from_ascii("HWECMC5X7A01");
    state->uds.did[AD_UDS_DID_system_supplier_ECU_hardware_version_number] = ad_buffer_from_ascii("HW01.00");
    state->uds.did[AD_UDS_DID_system_supplier_ECU_software_number] = ad_buffer_from_ascii("SWECMC5X7A01");
    state->uds.did[AD_UDS_DID_system_supplier_ECU_software_version_number] = ad_buffer_from_ascii("9666A1.00");
    state->uds.did[AD_UDS_DID_exhaust_regulation_type_approval_number] = ad_buffer_from_ascii("EU5");
    state->uds.did[AD_UDS_DID_system_name_engine_type] = ad_buffer_from_ascii("DW10CTED4");
    state->uds.did[AD_UDS_DID_repair_shop_code_tester_ad_serial_number] = ad_buffer_from_ascii("PSA-TOOL-000001");
    state->uds.did[AD_UDS_DID_programming_date] = ad_buffer_from_ascii("20110408");
    state->uds.did[AD_UDS_DID_ECU_installation_date] = ad_buffer_from_ascii("20110419");
    state->uds.did[AD_UDS_DID_ODX_file] = ad_buffer_from_ascii("CITROEN_C5X7_DW10C_ECM.ODX");

    for (i = 0; i < 0xFFFF; i++) {
        if (state->uds.did[i] == null) {
            state->uds.did[i] = ad_buffer_new_random_with_seed(10, seed);
        }
    }
}
static Buffer * response_uds_did(SimECUGenerator * generator, uint16_t did) {
    GState * state = (GState*)generator->state;
    unsigned * seed = generator->context;
    log_err("did = %d", did);
    ad_buffer_dump(state->uds.did[did]);
    return ad_buffer_copy(state->uds.did[did]);
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
        if ( ! ad_uds_service_allowed(state, binRequest->buffer[0]) ) {
            sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_CONDITIONS_NOT_CORRECT);
            return binResponse;
        }
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
            ad_list_DTC_clear(state->obd.dtcs);
            log_msg(LOG_DEBUG, "Clearing DTCs");
        } break;
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION: return generator->response_saej1979_vehicle_identification_request(generator, binRequest);
        case AD_UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL: {
            if ( 1 < binRequest->size ) {
                if ( binRequest->buffer[1] == AD_UDS_SESSION_DEFAULT ) {
                    ad_uds_reset_default_session(state);
                } else {
                    set_session_type(state, binRequest->buffer[1]);
                }
                ad_buffer_append_byte(binResponse, get_session_type(state));
                ad_buffer_append_melt(binResponse, ad_buffer_from_ints( 
                    0x00, 0x19, 0x07, 0xD0 
                ));
            }
        } break;
        case AD_UDS_SERVICE_READ_DATA_BY_IDENTIFIER: {
            binResponse = generator->response_uds_did_wrapper(generator, binRequest);
        } break;
        case AD_UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER: {
            if ( 3 < binRequest->size ) {
                uint16_t did = binRequest->buffer[1] << 8 | binRequest->buffer[2];
                ad_buffer_recycle(binResponse);
                ad_buffer_append_byte(binResponse, AD_UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER | AD_UDS_POSITIVE_RESPONSE);
                ad_buffer_append_uint16(binResponse, did);
                state->uds.did[did] = ad_buffer_slice(binRequest, 3, binRequest->size - 3);
                if ( did == AD_UDS_DID_VIN ) {
                    state->vin = ad_buffer_copy(state->uds.did[did]);
                }
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case AD_UDS_SERVICE_READ_DTC_INFORMATION: {
            if ( 1 < binRequest->size ) {
                ad_buffer_append_byte(binResponse, binRequest->buffer[1]);
                switch(binRequest->buffer[1]) {
                    case AD_UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_FIRST_CONFIRMED_DTC: {
                        ad_buffer_append_byte(binResponse, state->uds.DTCSupportedStatusMask);
                        for(int i = 0; i < state->uds.dtcs->size; i++) {
                            final AD_UDS_DTC * dtc = state->uds.dtcs->list[i];
                            ad_buffer_append_bytes(binResponse, dtc->data, DTC_DATA_SZ);
                            ad_buffer_append_byte(binResponse, dtc->status);
                        }
                    } break;
                    case AD_UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_DTC_BY_SEVERITY_MASK_RECORD: {
                        if ( binRequest->size <= 3 ) {
                            sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                        } else {
                            final byte DTCSeverityMask = binRequest->buffer[2];
                            final byte DTCStatusMask = binRequest->buffer[3];
                            ad_buffer_append_byte(binResponse, state->uds.DTCSupportedStatusMask);
                            for(int i = 0; i < state->uds.dtcs->size; i++) {
                                final AD_UDS_DTC * dtc = state->uds.dtcs->list[i];
                                ad_buffer_append_byte(binResponse, 0xFF); // DTCSeverity
                                ad_buffer_append_byte(binResponse, 0xFF); // DTCFunctionalUnit
                                ad_buffer_append_bytes(binResponse, dtc->data, DTC_DATA_SZ);
                                ad_buffer_append_byte(binResponse, dtc->status);
                            }
                        }
                    } break;
                    case AD_UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_DTC_BY_STATUS_MASK: {
                        if ( binRequest->size <= 2 ) {
                            sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                        } else {
                            final byte DTCStatusMask = binRequest->buffer[2];
                            ad_buffer_append_byte(binResponse, state->uds.DTCSupportedStatusMask);
                            for(int i = 0; i < state->uds.dtcs->size; i++) {
                                final AD_UDS_DTC * dtc = state->uds.dtcs->list[i];
                                if ( (dtc->status & DTCStatusMask) != 0 ) {
                                    ad_buffer_append_bytes(binResponse, dtc->data, DTC_DATA_SZ);
                                    ad_buffer_append_byte(binResponse, dtc->status);
                                }
                            }
                        }
                    } break;
                }
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case AD_UDS_SERVICE_TESTER_PRESENT: {
            if ( 1 < binRequest->size ) {
                switch(binRequest->buffer[1]) {
                    case AD_UDS_TESTER_PRESENT_SUB_ZERO:
                        ad_buffer_append_byte(binResponse, binRequest->buffer[1]);
                        break;
                    case AD_UDS_TESTER_PRESENT_SUB_NO_RESPONSE:
                        ad_buffer_append_byte(binResponse, binRequest->buffer[1]);
                        log_msg(LOG_DEBUG, "TODO (not sending anything back)");
                        break;
                    default:
                        log_msg(LOG_INFO, "unsupported sub function %02hhX", binRequest->buffer[1]);
                        sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
                        break;
                }
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case AD_UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION: {
            if ( 3 < binRequest->size ) {
                ad_list_DTC_clear((ad_list_DTC*)state->uds.dtcs);
                ad_list_DTC_clear(state->obd.dtcs);
                log_msg(LOG_DEBUG, "Should apply the group mask from the request");
                ad_buffer_slice_append(binResponse, binRequest, 1, 3);
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case AD_UDS_SERVICE_READ_MEMORY_BY_ADDRESS: {
            if ( 1 < binRequest->size ) {
                byte length = binRequest->buffer[1] >> 4;
                byte address_length = binRequest->buffer[1] & 0x0F;
                if ( ! ((2 + address_length) <= binRequest->size) ) {
                    sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                    break;
                }
                if ( ! ((2 + address_length + length) <= binRequest->size) ) {
                    sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                    break;
                }
                Buffer * memory_address = ad_buffer_slice(binRequest, 2, address_length);
                Buffer * memory_length = ad_buffer_slice(binRequest, 2 + address_length, length);
                uint64_t read_address = ad_buffer_to_be(memory_address);
                uint64_t read_length = ad_buffer_to_be(memory_length);
                if ( 0x0000 <= read_address && (read_address + read_length) <= GSTATE_UDS_MEMORY_SZ ) {
                    ad_buffer_slice_append(binResponse, state->uds.memory, read_address, read_length);
                } else {
                    sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_REQUEST_OUT_OF_RANGE);
                }
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case AD_UDS_SERVICE_WRITE_MEMORY_BY_ADDRESS: {
            if ( 1 < binRequest->size ) {
                byte formatLength = binRequest->buffer[1] >> 4;
                byte formatAddress = binRequest->buffer[1] & 0x0F;
                if ( ! ((2 + formatLength) <= binRequest->size) ) {
                    sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                    break;
                }
                if ( ! ((2 + formatLength + formatAddress) <= binRequest->size) ) {
                    sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                    break;
                }
                Buffer * memory_address = ad_buffer_slice(binRequest, 2, formatAddress);
                Buffer * memory_length = ad_buffer_slice(binRequest, 2 + formatAddress, formatLength);
                uint64_t write_address = ad_buffer_to_be(memory_address);
                uint64_t write_length = ad_buffer_to_be(memory_length);
                int dataRecordOffset = 2 + formatAddress + formatLength;
                Buffer * dataRecord = ad_buffer_slice(binRequest, dataRecordOffset, binRequest->size - dataRecordOffset);
                ad_buffer_assign_at(state->uds.memory, write_address, dataRecord);
                ad_buffer_slice_append(binResponse, binRequest, 1, dataRecordOffset - 1);
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case AD_UDS_SERVICE_SECURITY_ACCESS: {
            if ( 1 < binRequest->size ) {
                final int securit_seed = 0x4321;
                switch(binRequest->buffer[1]) {
                    case AD_UDS_SECURITY_ACCESS_ECU_GENERATOR_CITROEN_C5_X7_SEED: {
                        ad_buffer_append_byte(binResponse, binRequest->buffer[1]);
                        ad_buffer_append_byte(binResponse, securit_seed >> 8);
                        ad_buffer_append_byte(binResponse, securit_seed & 0xFF);
                    } break;
                    case AD_UDS_SECURITY_ACCESS_ECU_GENERATOR_CITROEN_C5_X7_KEY: {
                        assert(3 < binRequest->size);
                        int encrypted = binRequest->buffer[2] << 8 | binRequest->buffer[3];
                        int decrypted = ad_uds_security_access_ecu_generator_citroen_c5_x7_encrypt(encrypted);
                        log_msg(LOG_DEBUG, "From emu: encrypted received: 0x%X decrypted to 0x%X", encrypted, decrypted);
                        if ( securit_seed == decrypted ) {
                            ad_buffer_append_byte(binResponse, binRequest->buffer[1]);
                            state->uds.security_access_granted = true;
                        } else {
                            sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_SECURITY_ACCESS_DENIED);
                        }
                    } break;
                    default: {
                        sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
                    } break;
                }
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
            }
        } break;
        case AD_UDS_SERVICE_ECU_RESET: {
            if ( 1 < binRequest->size ) {
                final byte resetType = binRequest->buffer[1];
                ad_uds_reset_default_session(state);
                ad_buffer_append_byte(binResponse, resetType);
            } else {
                sim_ecu_generator_fill_nrc(binResponse, binRequest, AD_UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
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
    generator->response_saej1979_pid = response_saej1979_pid;
    generator->response_saej1979_dtcs = response_saej1979_dtcs;
    generator->response_saej1979_vehicle_identification_request_info_type = response_saej1979_vehicle_identification_request_info_type;
    generator->response_uds_did = response_uds_did;
    generator->state = (GState*)malloc(sizeof(GState));
    generator->context = (unsigned*)malloc(sizeof(unsigned));
    GState * state = (GState*)generator->state;
    state->vin = ad_buffer_from_ascii("VF1BB05CF26010203");
    state->uds.did[AD_UDS_DID_VIN] = ad_buffer_copy(state->vin);
    state->uds.memory = ad_buffer_new_random(GSTATE_UDS_MEMORY_SZ);
    *((unsigned *)generator->context) = 1;
    state->obd.dtcs = null;
    state->obd.mil_on = true;
    
    state->uds.security_access_granted = false;
    state->uds.dtcs = null;
    state->uds.DTCSupportedStatusMask = 
            AD_UDS_DTC_STATUS_TestFailed | AD_UDS_DTC_STATUS_TestFailedThisOperationCycle |
            AD_UDS_DTC_STATUS_PendingDTC | AD_UDS_DTC_STATUS_ConfirmedDTC |
            AD_UDS_DTC_STATUS_TestNotCompletedSinceLastClear | AD_UDS_DTC_STATUS_TestFailedSinceLastClear |
            AD_UDS_DTC_STATUS_TestNotCompletedThisOperationCycle | AD_UDS_DTC_STATUS_WarningIndicatorRequested;
    state->uds.session_timer = null;
    state->uds.session_continue = true;
    
    state->uds.dtcs = ad_list_AD_UDS_DTC_new();
    state->obd.dtcs = ad_list_DTC_new();

    ad_list_ad_object_string * dtcs = ad_list_ad_object_string_new();
    ad_list_ad_object_string_append(dtcs, ad_object_string_new_from("P0103"));
    ad_list_ad_object_string_append(dtcs, ad_object_string_new_from("P0104"));
    for(int i = 0; i < dtcs->size; i++) {
        SAEJ1979_DTC * dtc = saej1979_dtc_from_string(dtcs->list[i]->data);
        ad_list_DTC_append(state->obd.dtcs, (DTC*)dtc);
        ad_list_AD_UDS_DTC_append(state->uds.dtcs, AD_UDS_DTC_new_from(dtc));
    }
    init_did(state, generator->context);
    set_session_type(state, AD_UDS_SESSION_DEFAULT);
    return generator;
}