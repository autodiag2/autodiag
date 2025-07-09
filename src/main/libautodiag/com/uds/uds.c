#include "libautodiag/com/uds/uds.h"

list_Buffer * uds_read_data_by_identifier(final VehicleIFace * iface, final int did) {
    viface_lock(iface);
    list_Buffer * result = list_Buffer_new();
    char * request;
    asprintf(&request, "22%02hhX%02hhX", (did & 0xFF00) >> 8, did & 0xFF); // BIG endian with ELM
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
                buffer_slice(resultBuffer, data, 3, data->size - 4);
                list_Buffer_append(result, resultBuffer);
            }
        }
    }
    viface_unlock(iface);
    return result;
}