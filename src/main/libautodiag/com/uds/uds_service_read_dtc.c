#include "libautodiag/com/uds/uds_service_read_dtc.h"

object_UDS_DTC * object_UDS_DTC_new() { return UDS_DTC_new(); }
object_UDS_DTC * object_UDS_DTC_assign(object_UDS_DTC * to, object_UDS_DTC * from) {
    memcpy(to->data, from->data, DTC_DATA_SZ);
    to->description = from->description;
    to->ecu = from->ecu;
    to->status = from->status;
    return to;
}
void object_UDS_DTC_free(object_UDS_DTC *o) { return UDS_DTC_free(o); }
char * UDS_DTC_to_string(final UDS_DTC * dtc) {
    final Buffer * code = buffer_new();
    buffer_append_bytes(code, dtc->data, DTC_DATA_SZ);
    final byte type = (code->buffer[0] & 0xC0) >> 6;
    char letter = iso15031_dtc_type_first_letter(type);
    code->buffer[0] = (code->buffer[0] & (~0xC0)) ;
    code->size -= 1;
    char * result;
    asprintf(&result, "%c%s", letter, buffer_to_hex_string(code));
    buffer_free(code);
    return result;
}
UDS_DTC * UDS_DTC_new() {
    UDS_DTC * dtc = (UDS_DTC*)malloc(sizeof(UDS_DTC));
    dtc->status = UDS_DTC_STATUS_TestNotCompletedSinceLastClear | UDS_DTC_STATUS_TestNotCompletedThisOperationCycle;
    dtc->description = list_DTC_DESCRIPTION_new();
    dtc->to_string = CAST_DTC_TO_STRING(UDS_DTC_to_string);
    dtc->explanation = CAST_DTC_EXPLANATION(UDS_DTC_explanation);
    memset(dtc->data, 0x00, 3);
    dtc->ecu = null;
    return dtc;
}
char * UDS_DTC_explanation(final UDS_DTC * dtc) {
    char * result = strdup("This code has been detected with UDS");
    for(int i = 1; i < 256; i *= 2) {
        char * status_string = uds_dtc_status_to_string(i);
        assert(status_string != null);
        char * r2;
        asprintf(&r2, "%s%s%s: %d", result, strlen(result) == 0 ? "" : "\n", 
            status_string, (dtc->status & i) != 0
        );
        free(result);
        free(status_string);
        result = r2;
    }
    return result;
}
UDS_DTC * UDS_DTC_new_from(final SAEJ1979_DTC *dtc) {
    UDS_DTC * udtc = UDS_DTC_new();
    memcpy(udtc->data, dtc->data, DTC_DATA_SZ);
    udtc->ecu = dtc->ecu;
    return udtc;
}
void UDS_DTC_free(UDS_DTC * dtc) {
    if ( dtc->description != null ) {
        list_DTC_DESCRIPTION_free(dtc->description);
        dtc->description = null;
    }
    dtc->ecu = null;
    free(dtc);
}
int UDS_DTC_cmp(final UDS_DTC * e1, final UDS_DTC * e2) {
    return memcmp(e1->data, e2->data, DTC_DATA_SZ);
}
AD_LIST_SRC_DEEP(UDS_DTC,
    list->Status_Availability_Mask = 0xFF;
    list->ecu = null;
    ,
    list->Status_Availability_Mask = 0x00;
    list->ecu = null;
)
int list_UDS_DTC_cmp(final list_UDS_DTC * e1, final list_UDS_DTC * e2) {
    return e1 - e2;
}
AD_LIST_SRC(list_UDS_DTC);

static list_list_UDS_DTC * uds_read_dtcs_with_mask(final VehicleIFace * iface, final Vehicle * filter, final UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION sub, final byte StatusMask) {
    list_list_UDS_DTC * result = list_list_UDS_DTC_new();
    viface_lock(iface);
    char * request;
    asprintf(&request, "%02hhX%02hhX", UDS_SERVICE_READ_DTC_INFORMATION, sub);
    if ( sub == UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_DTC_BY_STATUS_MASK ) {
        char * request2;
        asprintf(&request2, "%s%02hhX", request, StatusMask);
        free(request);
        request = request2;
    }
    viface_send(iface, request);
    free(request);
    viface_clear_data(iface);
    viface_recv(iface);
    for(int i = 0; i < iface->vehicle->ecus_len; i++) {
        final ECU * ecu = iface->vehicle->ecus[i];
        final list_UDS_DTC * ecu_response = list_UDS_DTC_new();
        ecu_response->ecu = ecu;
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            final Buffer * data = ecu->data_buffer->list[j];
            if ( data->size < 3 ) {
                log_msg(LOG_WARNING, "received some data with wrong size (probably concurrent access)");
                buffer_dump(data);
                continue;
            }
            if ( data->buffer[0] == UDS_NEGATIVE_RESPONSE ) {
                log_msg(LOG_DEBUG, "negative response found: ");
                buffer_dump(data);
            } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                if ( (data->buffer[1] & sub) == sub ) {
                    ecu_response->Status_Availability_Mask = data->buffer[2];
                    for(int i = 3; i < (data->size-DTC_DATA_SZ); i += (DTC_DATA_SZ+1)) {
                        final UDS_DTC * dtc = UDS_DTC_new();
                        memcpy(dtc->data, data->buffer + i, DTC_DATA_SZ);
                        dtc->status = data->buffer[i+DTC_DATA_SZ];
                        dtc->ecu = ecu;
                        list_UDS_DTC_append(ecu_response, dtc);
                        dtc_description_fetch_from_fs(dtc, filter);
                    }
                }
            } else {
                log_msg(LOG_WARNING, "Unknown byte at first");
                buffer_dump(data);
            }
        }
        list_list_UDS_DTC_append(result, ecu_response);
    }
    viface_unlock(iface);
    return result;
}
list_list_UDS_DTC * uds_read_dtc_by_status_mask(final VehicleIFace * iface, final Vehicle * filter, final byte StatusMask) {
    return uds_read_dtcs_with_mask(iface, filter, UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_DTC_BY_STATUS_MASK, StatusMask);
}
list_list_UDS_DTC * uds_read_dtc_first_confirmed_dtc(final VehicleIFace * iface, final Vehicle * filter) {
    return uds_read_dtcs_with_mask(iface, filter, UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_FIRST_CONFIRMED_DTC, 0x00);
}
list_UDS_DTC * uds_read_all_dtcs(final VehicleIFace * iface, final Vehicle * filter) {
    list_list_UDS_DTC * lists_dtcs = uds_read_dtc_by_status_mask(iface, filter, 0x00);
    list_UDS_DTC * dtcs = list_UDS_DTC_new();
    for(int i = 0; i < lists_dtcs->size; i ++) {
        list_DTC_append_list(dtcs, lists_dtcs->list[0]);
    }
    return dtcs;
}
void uds_dtc_dump(final UDS_DTC * dtc) {
    log_msg(LOG_DEBUG, "%s", dtc->to_string(dtc));
}

char * uds_dtc_status_to_string(UDS_DTC_STATUS wanted) {
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