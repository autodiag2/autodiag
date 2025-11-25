#include "libautodiag/com/uds/uds_service_read_dtc.h"

list_Buffer * uds_read_dtc_first_confirmed_dtc(final VehicleIFace * iface) {
    list_Buffer * result = list_Buffer_new();
    viface_lock(iface);
    char * request;
    asprintf(&request, "%02hhX%02hhX%02hhX", UDS_SERVICE_READ_DTC_INFORMATION, UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_FIRST_CONFIRMED_DTC);
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
                final byte DTC_Status_Availability_Mask = data->buffer[2];
                for(int i = 3; i < (data->size-3); i += 4) {
                    Buffer * resultBuffer = buffer_new();
                    buffer_slice_append(resultBuffer, data, i, 4);
                    list_Buffer_append(result, resultBuffer);
                }
            } else {
                log_msg(LOG_WARNING, "Unknown byte at first");
                buffer_dump(data);
            }
        }
    }
    viface_unlock(iface);
    return result;
}