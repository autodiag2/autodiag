#include "libautodiag/com/uds/uds.h"

list_Buffer * uds_read_data_by_identifier(final VehicleIFace * iface, final int did) {
    viface_lock(iface);
    list_Buffer * result = list_Buffer_new();
    char * request;
    asprintf(&request, "%02hhX%02hhX%02hhX", UDS_SERVICE_READ_DATA_BY_IDENTIFIER, (did & 0xFF00) >> 8, did & 0xFF); // BIG endian with ELM
    viface_send(iface, request);
    viface_clear_data(iface);
    viface_recv(iface);
    for(int i = 0; i < iface->vehicle->ecus_len; i++) {
        final ECU * ecu = iface->vehicle->ecus[i];
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            final Buffer * data = ecu->data_buffer->list[j];
            if ( data->buffer[0] == uds_service_response(UDS_SERVICE_READ_DATA_BY_IDENTIFIER) ) {
                final int received_did = data->buffer[1] << 8 | data->buffer[2];
                if ( did != received_did ) {
                    log_msg(LOG_ERROR, "received did do not match the sent one, ignoring");
                    continue;
                }
                Buffer * resultBuffer = buffer_new();
                buffer_slice_append(resultBuffer, data, 3, data->size - 4);
                list_Buffer_append(result, resultBuffer);
            }
        }
    }
    viface_unlock(iface);
    return result;
}
bool uds_request_session_cond(final VehicleIFace * iface, final UDS_SESSION session_type) {
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

object_hashmap_Int_Int * uds_request_session(final VehicleIFace * iface, final UDS_SESSION session_type) {
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
            if ( ( ( data->buffer[0] & UDS_NEGATIVE_RESPONSE ) != UDS_NEGATIVE_RESPONSE ) ) {
                object_hashmap_Int_Int_set(
                    result,
                    intdup(ecu->address->buffer[1]), 
                    booldup(false)
                );
            } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) != 0 ) {
                object_hashmap_Int_Int_set(
                    result,
                    intdup(ecu->address->buffer[1]), 
                    booldup(true)
                );
            } else {
                log_msg(LOG_ERROR, "Incorrect data byte: %02hhX", data->buffer[0]);
            }
        }
    }
    viface_unlock(iface);
    return result;
}