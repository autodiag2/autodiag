#include "libautodiag/com/uds/uds.h"

bool ad_uds_write_vin(final VehicleIFace * iface, final Buffer * vin_ascii) {
    assert(vin_ascii != null);
    bool * result = null;

    if ( ! ad_uds_request_session_cond(iface, UDS_SESSION_PROGRAMMING) ) {
        return false;
    }

    if ( ! ad_uds_security_access(iface, 0) ) {
        return false;
    }

    viface_lock(iface);

    Buffer * request = ad_buffer_new();
    ad_buffer_append_byte(request, AD_UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER);
    Buffer * did = ad_buffer_new();
    ad_buffer_assign_uint16(did, UDS_DID_VIN);
    ad_buffer_append(request, did);
    ad_buffer_append(request, vin_ascii);

    viface_send(iface, request);
    ad_buffer_free(request);

    viface_clear_data(iface);
    viface_recv(iface);

    for(int i = 0; i < iface->vehicle->ecus->size; i++) {
        final ad_object_ECU * ecu = iface->vehicle->ecus->list[i];
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            final Buffer * data = ecu->data_buffer->list[j];

            if ( result == null ) {
                result = booldup(true);
            }

            if ( data->size <= 0 ) {
                log_msg(LOG_WARNING, "Empty response buffer");
                *result &= false;
                continue;
            }

            if ( data->buffer[0] == UDS_NEGATIVE_RESPONSE ) {
                log_msg(LOG_ERROR, "Negative response while writing VIN");
                ad_buffer_dump(data);
                *result &= false;
            } else if ( data->size == 3
                     && data->buffer[0] == (AD_UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER | UDS_POSITIVE_RESPONSE)
                     && data->buffer[1] == did->buffer[0]
                     && data->buffer[2] == did->buffer[1] ) {
                *result &= true;
            } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                log_msg(LOG_WARNING, "Unexpected positive response payload while writing VIN");
                ad_buffer_dump(data);
                *result &= false;
            } else {
                log_msg(LOG_WARNING, "Unknown byte at first");
                ad_buffer_dump(data);
                *result &= false;
            }
        }
    }
    ad_buffer_free(did);
    viface_unlock(iface);
    return result == null ? false : *result;
}
bool ad_uds_security_access(final VehicleIFace * iface, int level) {
    return ad_uds_security_access_ecu_generator_citroen_c5_x7(iface);
}
const char *ad_uds_reset_type_to_string(ad_uds_reset_type v) {
    if (v == UDS_RESET_HARD) return "Hard Reset";
    if (v == UDS_RESET_KEY_OFF_ON) return "Key Off On Reset";
    if (v == UDS_RESET_SOFT) return "Soft Reset";
    if (v == UDS_RESET_ENABLE_RPSD) return "Enable Rapid Power Shut Down";
    if (v == UDS_RESET_DISABLE_RPSD) return "Disable Rapid Power Shut Down";
    if (v == UDS_RESET_RESERVED_07) return "ISO/SAE Reserved";

    if (0x06 < v && v < 0x40) return "ISO/SAE Reserved";
    if (0x40 <= v && v < 0x60) return "OEM Specific";
    if (0x60 <= v && v < 0x7F) return "System Supplier Specific";

    return "Undefined";
}

bool ad_uds_reset_ecu(final VehicleIFace * iface, final ad_uds_reset_type type) {
    bool * result = null;
    if ( ! ad_uds_request_session_cond(iface, UDS_SESSION_PROGRAMMING) ) {
        return false;
    }
    viface_lock(iface);
    viface_send(iface, ad_buffer_from_ints( 
        AD_UDS_SERVICE_ECU_RESET, type
    ));
    viface_clear_data(iface);
    viface_recv(iface);
    for(int i = 0; i < iface->vehicle->ecus->size; i++) {
        final ad_object_ECU * ecu = iface->vehicle->ecus->list[i];
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            final Buffer * data = ecu->data_buffer->list[j];
            if ( result == null ) {
                result = booldup(true);
            }
            if ( data->buffer[0] == UDS_NEGATIVE_RESPONSE ) {
                log_msg(LOG_DEBUG, "negative response found: ");
                ad_buffer_dump(data);
                *result &= false;
            } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                *result &= true;
            } else {
                log_msg(LOG_WARNING, "Unknown byte at first");
                ad_buffer_dump(data);
            }
        }
    }
    viface_unlock(iface);
    return result == null ? false : *result;
}

bool ad_uds_clear_dtcs(final VehicleIFace * iface) {
    bool *result = null;
    if ( ! ad_uds_request_session_cond(iface, UDS_SESSION_EXTENDED_DIAGNOSTIC) ) {
        return false;
    }
    final byte emissionRelatedDTC = 0xFF;
    final byte group2RelatedDTC = 0xFF;
    final byte group3RelatedDTC = 0xFF;
    viface_lock(iface);
    viface_send(iface, ad_buffer_from_ints( 
        AD_UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION, emissionRelatedDTC, group2RelatedDTC, group3RelatedDTC
    ));
    viface_clear_data(iface);
    viface_recv(iface);
    for(int i = 0; i < iface->vehicle->ecus->size; i++) {
        final ad_object_ECU * ecu = iface->vehicle->ecus->list[i];
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            final Buffer * data = ecu->data_buffer->list[j];
            if ( result == null ) {
                result = booldup(true);
            }
            if ( data->buffer[0] == UDS_NEGATIVE_RESPONSE ) {
                log_msg(LOG_DEBUG, "negative response found: ");
                ad_buffer_dump(data);
                *result &= false;
            } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                *result &= true;
            } else {
                log_msg(LOG_WARNING, "Unknown byte at first");
                ad_buffer_dump(data);
            }
        }
    }
    viface_unlock(iface);
    return result == null ? false : *result;
}
ad_list_Buffer * ad_uds_read_data_by_identifier(final VehicleIFace * iface, final int did) {
    ad_list_Buffer * result = ad_list_Buffer_new();
    viface_lock(iface);
    final Buffer * binRequest = ad_buffer_from_ints(AD_UDS_SERVICE_READ_DATA_BY_IDENTIFIER, (did & 0xFF00) >> 8, did & 0xFF);
    viface_send(iface, binRequest);
    ad_buffer_free(binRequest);
    viface_clear_data(iface);
    viface_recv(iface);
    for(int i = 0; i < iface->vehicle->ecus->size; i++) {
        final ad_object_ECU * ecu = iface->vehicle->ecus->list[i];
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            final Buffer * data = ecu->data_buffer->list[j];
            if ( data->buffer[0] == UDS_NEGATIVE_RESPONSE ) {
                log_msg(LOG_DEBUG, "negative response found: ");
                ad_buffer_dump(data);
            } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                final int received_did = data->buffer[1] << 8 | data->buffer[2];
                if ( did != received_did ) {
                    log_msg(LOG_ERROR, "received did do not match the sent one, ignoring");
                    continue;
                }
                Buffer * resultBuffer = ad_buffer_new();
                ad_buffer_slice_append(resultBuffer, data, 3, data->size - 3);
                ad_list_Buffer_append(result, resultBuffer);
            } else {
                log_msg(LOG_WARNING, "Unknown byte at first");
                ad_buffer_dump(data);
            }
        }
    }
    viface_unlock(iface);
    return result;
}
bool ad_uds_is_enabled(final VehicleIFace *iface) {
    return ad_uds_request_session_cond(iface, UDS_SESSION_DEFAULT);
}
bool ad_uds_request_session_cond(final VehicleIFace * iface, final byte session_type) {
    final ad_object_hashmap_Int_Int * result = ad_uds_request_session(iface, session_type);
    int value = -1;
    if ( 0 < result->size ) {
        for(int i = 0; i < result->size; i++) {
            if ( value != -1 && result->values[i]->value != value ) {
                log_msg(LOG_WARNING, "Multiple ECU programming is not supported at the time");
                return true;
            }
            value = result->values[i]->value;
        }
    }
    ad_object_hashmap_Int_Int_free(result);
    return value == -1 ? false : value;
}

bool ad_uds_tester_present(final VehicleIFace *iface, final bool response) {
    bool result = true;
    viface_lock(iface);
    viface_send(iface, ad_buffer_from_ints( AD_UDS_SERVICE_TESTER_PRESENT, response ? UDS_TESTER_PRESENT_SUB_ZERO : UDS_TESTER_PRESENT_SUB_NO_RESPONSE));
    if ( response ) {
        viface_clear_data(iface);
        if ( viface_recv(iface) <= 0 ) {
            log_msg(LOG_ERROR, "Tester present response not received");
            viface_unlock(iface);
            return false;
        }
        for(int i = 0; i < iface->vehicle->ecus->size; i++) {
            final ad_object_ECU * ecu = iface->vehicle->ecus->list[i];
            for(int j = 0; j < ecu->data_buffer->size; j++) {
                final Buffer * ecu_buffer = ecu->data_buffer->list[j];
                if ( ecu_buffer->buffer[0] == UDS_NEGATIVE_RESPONSE ) {
                    result &= false;
                } else if ( (ecu_buffer->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                    result &= true;
                } else {
                    log_msg(LOG_ERROR, "nor positive nor negative response received: %s", ad_buffer_to_hex_string(ecu_buffer));
                    result &= false;
                }
            }
        }
    }
    viface_unlock(iface);
    return result;
}

static void * tester_present_timer_daemon(void *arg) {
    final VehicleIFace * iface = (VehicleIFace*)arg;
    while(true) {
        if ( iface->connection._state == VIFaceState_NOT_READY || iface->uds.tester_present_timer == null ) {
            break;
        } else {
            if ( ! ad_uds_tester_present(iface, true) ) {
                log_msg(LOG_ERROR, "Periodic message tester present not sent, session will reset to default");
                break;
            }
            usleep((UDS_SESSION_TIMEOUT_MS * 1000) * 3/4);
        }
    }
    log_msg(LOG_DEBUG, "Terminating the beacon thread");
    if ( iface->uds.tester_present_timer != null ) {
        free(iface->uds.tester_present_timer);
        iface->uds.tester_present_timer = null;
    }
    return null;
}
void ad_uds_viface_start_tester_present_timer(final VehicleIFace * iface) {
    if ( iface->uds.tester_present_timer == null ) {
        iface->uds.tester_present_timer = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(iface->uds.tester_present_timer, null, tester_present_timer_daemon, (void*)iface);
    }
}
void ad_uds_viface_stop_tester_present_timer(final VehicleIFace * iface) {
    if ( iface->uds.tester_present_timer != null ) {
        pthread_t thread = *(iface->uds.tester_present_timer);
        free(iface->uds.tester_present_timer);
        iface->uds.tester_present_timer = null;
        pthread_join(thread, null);
    }
}
int ad_uds_security_access_ecu_generator_citroen_c5_x7_encrypt(int seed) {
    return seed ^ UDS_SECURITY_ACCESS_ECU_GENERATOR_CITROEN_C5_X7_PRIVATE_KEY;
}
bool ad_uds_security_access_ecu_generator_citroen_c5_x7(final VehicleIFace * iface) {
    final ad_object_hashmap_Int_Int * seeds = ad_object_hashmap_Int_Int_new();
    viface_lock(iface);
    viface_send(iface, ad_buffer_from_ints( 
        AD_UDS_SERVICE_SECURITY_ACCESS, UDS_SECURITY_ACCESS_ECU_GENERATOR_CITROEN_C5_X7_SEED
    ));
    viface_clear_data(iface);
    viface_recv(iface);
    for(int i = 0; i < iface->vehicle->ecus->size; i++) {
        final ad_object_ECU * ecu = iface->vehicle->ecus->list[i];
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            final Buffer * data = ecu->data_buffer->list[j];
            if ( 0 == data->size ) {
                log_msg(LOG_WARNING, "Empty response buffer");
                continue;
            }
            if ( ( ( data->buffer[0] & UDS_NEGATIVE_RESPONSE ) == UDS_NEGATIVE_RESPONSE ) ) {
                log_msg(LOG_ERROR, "ECU has responded negatively to seed request");
            } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                if ( data->size < 4 ) {
                    log_msg(LOG_ERROR, "Incorrect seed response size, expected at least 4 bytes but got %d", data->size);
                    continue;
                }
                if ( data->buffer[1] != UDS_SECURITY_ACCESS_ECU_GENERATOR_CITROEN_C5_X7_SEED ) {
                    log_warn("incorrect response");
                    continue;
                }
                ad_object_hashmap_Int_Int_set(
                    seeds,
                    ad_object_Int_new_from(ecu->address->buffer[1]), 
                    ad_object_Int_new_from(data->buffer[2] << 8 | data->buffer[3])
                );
                ad_uds_viface_start_tester_present_timer(iface);
            } else {
                log_msg(LOG_ERROR, "Incorrect data byte: %02hhX", data->buffer[0]);
            }
        }
    }
    final ad_object_hashmap_Int_Int * verify = ad_object_hashmap_Int_Int_new();
    for(int i = 0; i < seeds->size; i++) {
        final int encrypted = ad_uds_security_access_ecu_generator_citroen_c5_x7_encrypt(seeds->values[i]->value);
        viface_send(iface, ad_buffer_from_ints(
            AD_UDS_SERVICE_SECURITY_ACCESS, UDS_SECURITY_ACCESS_ECU_GENERATOR_CITROEN_C5_X7_KEY,
            (encrypted & 0xFF00) >> 8,
            encrypted & 0x00FF
        ));
        viface_clear_data(iface);
        viface_recv(iface);
        for(int i = 0; i < iface->vehicle->ecus->size; i++) {
            final ad_object_ECU * ecu = iface->vehicle->ecus->list[i];
            for(int j = 0; j < ecu->data_buffer->size; j++) {
                final Buffer * data = ecu->data_buffer->list[j];
                if ( 0 == data->size ) {
                    log_msg(LOG_WARNING, "Empty response buffer");
                    continue;
                }
                if ( ( ( data->buffer[0] & UDS_NEGATIVE_RESPONSE ) == UDS_NEGATIVE_RESPONSE ) ) {
                    log_msg(LOG_ERROR, "ECU has responded negatively to verify key request");
                    ad_object_hashmap_Int_Int_set(
                        verify,
                        ad_object_Int_new_from(ecu->address->buffer[1]), 
                        ad_object_Int_new_from(false)
                    );
                } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                    if ( data->size < 2 ) {
                        log_msg(LOG_ERROR, "Incorrect verify key response size, expected at least 2 bytes but got %d", data->size);
                        continue;
                    }
                    if ( data->buffer[1] != UDS_SECURITY_ACCESS_ECU_GENERATOR_CITROEN_C5_X7_KEY ) {
                        log_warn("incorrect response");
                        continue;
                    }
                    ad_object_hashmap_Int_Int_set(
                        verify,
                        ad_object_Int_new_from(ecu->address->buffer[1]), 
                        ad_object_Int_new_from(true)
                    );
                    ad_uds_viface_start_tester_present_timer(iface);
                } else {
                    log_msg(LOG_ERROR, "Incorrect data byte: %02hhX", data->buffer[0]);
                }
            }
        }
    }
    viface_unlock(iface);
    bool * result = null;
    for(int i = 0; i < verify->size; i++) {
        if ( result == null ) {
            result = booldup(verify->values[i]->value);
        } else {
            *result &= verify->values[i]->value; 
        }
    }
    ad_object_hashmap_Int_Int_free(verify);
    ad_object_hashmap_Int_Int_free(seeds);
    return result == null ? false : *result;
}
ad_object_hashmap_Int_Int * ad_uds_request_session(final VehicleIFace * iface, final byte session_type) {
    viface_lock(iface);
    
    final ad_object_hashmap_Int_Int * result = ad_object_hashmap_Int_Int_new();
    final Buffer * binRequest = ad_buffer_from_ints(AD_UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL, session_type);
    viface_send(iface, binRequest);
    ad_buffer_free(binRequest);
    viface_clear_data(iface);
    viface_recv(iface);
    for(int i = 0; i < iface->vehicle->ecus->size; i++) {
        final ad_object_ECU * ecu = iface->vehicle->ecus->list[i];
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            final Buffer * data = ecu->data_buffer->list[j];
            if ( ( ( data->buffer[0] & UDS_NEGATIVE_RESPONSE ) == UDS_NEGATIVE_RESPONSE ) ) {
                ad_object_hashmap_Int_Int_set(
                    result,
                    ad_object_Int_new_from(ecu->address->buffer[1]), 
                    ad_object_Int_new_from(false)
                );
            } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                ad_object_hashmap_Int_Int_set(
                    result,
                    ad_object_Int_new_from(ecu->address->buffer[1]), 
                    ad_object_Int_new_from(true)
                );
                ad_uds_viface_start_tester_present_timer(iface);
            } else {
                log_msg(LOG_ERROR, "Incorrect data byte: %02hhX", data->buffer[0]);
            }
        }
    }
    viface_unlock(iface);
    return result;
}