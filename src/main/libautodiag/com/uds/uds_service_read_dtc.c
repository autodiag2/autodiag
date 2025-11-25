#include "libautodiag/com/uds/uds_service_read_dtc.h"

UDS_DTC * UDS_DTC_new() {
    UDS_DTC * result = (UDS_DTC*)malloc(sizeof(UDS_DTC));
    result->status = 0xFF;
    result->description = null;
    memset(result->data, 0x00, 3);
    return result;
}
void UDS_DTC_free(UDS_DTC * dtc) {
    free(dtc);
}
int UDS_DTC_cmp(final UDS_DTC * e1, final UDS_DTC * e2) {
    return memcmp(e1->data, e2->data, DTC_DATA_SZ);
}
AD_LIST_SRC_DEEP(UDS_DTC,
    list->Status_Availability_Mask = 0xFF;
    ,
    list->Status_Availability_Mask = 0x00;
)
int list_UDS_DTC_cmp(final list_UDS_DTC * e1, final list_UDS_DTC * e2) {
    return e1 - e2;
}
AD_LIST_SRC(list_UDS_DTC);

list_list_UDS_DTC * uds_read_dtc_first_confirmed_dtc(final VehicleIFace * iface) {
    list_list_UDS_DTC * result = list_list_UDS_DTC_new();
    viface_lock(iface);
    char * request;
    asprintf(&request, "%02hhX%02hhX%02hhX", UDS_SERVICE_READ_DTC_INFORMATION, UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_FIRST_CONFIRMED_DTC);
    viface_send(iface, request);
    free(request);
    viface_clear_data(iface);
    viface_recv(iface);
    for(int i = 0; i < iface->vehicle->ecus_len; i++) {
        final ECU * ecu = iface->vehicle->ecus[i];
        final list_UDS_DTC * ecu_response = list_UDS_DTC_new();
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            final Buffer * data = ecu->data_buffer->list[j];
            if ( data->buffer[0] == UDS_NEGATIVE_RESPONSE ) {
                log_msg(LOG_DEBUG, "negative response found: ");
                buffer_dump(data);
            } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                ecu_response->Status_Availability_Mask = data->buffer[2];
                for(int i = 3; i < (data->size-DTC_DATA_SZ); i += (DTC_DATA_SZ+1)) {
                    final UDS_DTC * dtc = UDS_DTC_new();
                    memcpy(dtc->data, data->buffer + i, DTC_DATA_SZ);
                    dtc->status = data->buffer[i+DTC_DATA_SZ];
                    list_UDS_DTC_append(ecu_response, dtc);
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

char * uds_dtc_status_to_string(byte status, UDS_DTC_STATUS wanted) {
    if ((status & wanted) == 0) return null;
    switch (wanted) {
        case UDS_DTC_STATUS_TestFailed:
            return strdup("Test Failed");
        case UDS_DTC_STATUS_TestFailedThisOperationCycle:
            return strdup("Test Failed This Operation Cycle");
        case UDS_DTC_STATUS_PendingDTC:
            return strdup("Pending DTC");
        case UDS_DTC_STATUS_ConfirmedDTC:
            return strdup("Confirmed DTC");
        case UDS_DTC_STATUS_TestNotCompletedSinceLastClear:
            return strdup("Test Not Completed Since Last Clear");
        case UDS_DTC_STATUS_TestFailedSinceLastClear:
            return strdup("Test Failed Since Last Clear");
        case UDS_DTC_STATUS_TestNotCompletedThisOperationCycle:
            return strdup("Test Not Completed This Operation Cycle");
        case UDS_DTC_STATUS_WarningIndicatorRequested:
            return strdup("Warning Indicator Requested");
    }
    return null;
}