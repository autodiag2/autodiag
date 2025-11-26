#include "libautodiag/com/uds/uds.h"

list_Buffer * uds_read_data_by_identifier(final VehicleIFace * iface, final int did) {
    list_Buffer * result = list_Buffer_new();
    viface_lock(iface);
    char * request;
    asprintf(&request, "%02hhX%02hhX%02hhX", UDS_SERVICE_READ_DATA_BY_IDENTIFIER, (did & 0xFF00) >> 8, did & 0xFF);
    viface_send(iface, request);
    free(request);
    viface_clear_data(iface);
    viface_recv(iface);
    for(int i = 0; i < iface->vehicle->ecus_len; i++) {
        final ECU * ecu = iface->vehicle->ecus[i];
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            final Buffer * data = ecu->data_buffer->list[j];
            if ( data->buffer[0] == UDS_NEGATIVE_RESPONSE ) {
                log_msg(LOG_DEBUG, "negative response found: ");
                buffer_dump(data);
            } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                final int received_did = data->buffer[1] << 8 | data->buffer[2];
                if ( did != received_did ) {
                    log_msg(LOG_ERROR, "received did do not match the sent one, ignoring");
                    continue;
                }
                Buffer * resultBuffer = buffer_new();
                buffer_slice_append(resultBuffer, data, 3, data->size - 3);
                list_Buffer_append(result, resultBuffer);
            } else {
                log_msg(LOG_WARNING, "Unknown byte at first");
                buffer_dump(data);
            }
        }
    }
    viface_unlock(iface);
    return result;
}
bool uds_is_enabled(final VehicleIFace *iface) {
    return uds_request_session_cond(iface, UDS_SESSION_DEFAULT);
}
bool uds_request_session_cond(final VehicleIFace * iface, final byte session_type) {
    final object_hashmap_Int_Int * result = uds_request_session(iface, session_type);
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
    object_hashmap_Int_Int_free(result);
    return value == -1 ? false : value;
}

bool uds_tester_present(final VehicleIFace *iface, final bool response) {
    bool result = true;
    viface_lock(iface);
    viface_send(iface, gprintf("%02hhX%02hhx", UDS_SERVICE_TESTER_PRESENT, response ? UDS_TESTER_PRESENT_SUB_ZERO : UDS_TESTER_PRESENT_SUB_NO_RESPONSE));
    if ( response ) {
        viface_clear_data(iface);
        viface_recv(iface);
        for(int i = 0; i < iface->vehicle->ecus_len; i++) {
            final ECU * ecu = iface->vehicle->ecus[i];
            for(int j = 0; j < ecu->data_buffer->size; j++) {
                final Buffer * ecu_buffer = ecu->data_buffer->list[j];
                if ( ecu_buffer->buffer[0] == UDS_NEGATIVE_RESPONSE ) {
                    result &= false;
                } else if ( (ecu_buffer->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                    result &= true;
                } else {
                    log_msg(LOG_ERROR, "nor positive nor negative response received: %s", buffer_to_hex_string(ecu_buffer));
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
        if ( iface->state == VIFaceState_NOT_READY || iface->uds_tester_present_timer == null ) {
            break;
        } else {
            if ( ! uds_tester_present(iface, true) ) {
                log_msg(LOG_ERROR, "Periodic message tester present not sent, session will reset to default");
                break;
            }
            usleep((UDS_SESSION_TIMEOUT_MS * 1000) * 3/4);
        }
    }
    log_msg(LOG_DEBUG, "Terminating the beacon thread");
    if ( iface->uds_tester_present_timer != null ) {
        free(iface->uds_tester_present_timer);
        iface->uds_tester_present_timer = null;
    }
    return null;
}
void uds_viface_start_tester_present_timer(final VehicleIFace * iface) {
    if ( iface->uds_tester_present_timer == null ) {
        iface->uds_tester_present_timer = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(iface->uds_tester_present_timer, null, tester_present_timer_daemon, (void*)iface);
    }
}
void uds_viface_stop_tester_present_timer(final VehicleIFace * iface) {
    if ( iface->uds_tester_present_timer != null ) {
        free(iface->uds_tester_present_timer);
        iface->uds_tester_present_timer = null;
    }
}
object_hashmap_Int_Int * uds_request_session(final VehicleIFace * iface, final byte session_type) {
    viface_lock(iface);
    
    char * request;
    final object_hashmap_Int_Int * result = object_hashmap_Int_Int_new();
    asprintf(&request, "%02hhX%02hhX", UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL, session_type);
    viface_send(iface, request);
    viface_clear_data(iface);
    viface_recv(iface);
    for(int i = 0; i < iface->vehicle->ecus_len; i++) {
        final ECU * ecu = iface->vehicle->ecus[i];
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            final Buffer * data = ecu->data_buffer->list[j];
            if ( ( ( data->buffer[0] & UDS_NEGATIVE_RESPONSE ) == UDS_NEGATIVE_RESPONSE ) ) {
                object_hashmap_Int_Int_set(
                    result,
                    object_Int_new_from(ecu->address->buffer[1]), 
                    object_Int_new_from(false)
                );
            } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                object_hashmap_Int_Int_set(
                    result,
                    object_Int_new_from(ecu->address->buffer[1]), 
                    object_Int_new_from(true)
                );
                uds_viface_start_tester_present_timer(iface);
            } else {
                log_msg(LOG_ERROR, "Incorrect data byte: %02hhX", data->buffer[0]);
            }
        }
    }
    viface_unlock(iface);
    return result;
}