#include "libautodiag/com/uds/uds_service_read_dtc.h"

AD_OBJECT_SRC(UDS_DTC)

ad_object_UDS_DTC * ad_object_UDS_DTC_new() { return UDS_DTC_new(); }
ad_object_UDS_DTC * ad_object_UDS_DTC_assign(ad_object_UDS_DTC * to, ad_object_UDS_DTC * from) {
    memcpy(to->data, from->data, DTC_DATA_SZ);
    to->description = from->description;
    to->ecu = from->ecu;
    to->status = from->status;
    return to;
}
void ad_object_UDS_DTC_free(ad_object_UDS_DTC *o) { return UDS_DTC_free(o); }
char * UDS_DTC_to_string(final UDS_DTC * dtc) {
    return saej1979_dtc_to_string((DTC*)dtc);
}
static bool from_string(DTC * dtc, char * str) {
    SAEJ1979_DTC * decoded = saej1979_dtc_from_string(str);
    if ( decoded == null ) {
        return false;
    }
    memcpy(dtc->data, decoded->data, DTC_DATA_SZ);
    return true;
}
UDS_DTC * UDS_DTC_new() {
    UDS_DTC * dtc = (UDS_DTC*)malloc(sizeof(UDS_DTC));
    dtc->status = UDS_DTC_STATUS_TestNotCompletedSinceLastClear | UDS_DTC_STATUS_TestNotCompletedThisOperationCycle;
    dtc->description = ad_list_DTC_DESCRIPTION_new();
    dtc->detection_method = ad_list_ad_object_string_new();
    ad_list_ad_object_string_append(dtc->detection_method, ad_object_string_new_from("UDS"));
    dtc->to_string = AD_DTC_TO_STRING(UDS_DTC_to_string);
    dtc->from_string = AD_DTC_FROM_STRING(from_string);
    memset(dtc->data, 0x00, 3);
    dtc->ecu = null;
    return dtc;
}
char * UDS_DTC_explanation(final UDS_DTC * dtc) {
    char * result = strdup("ISO14229 (UDS):");
    for(int i = 1; i < 256; i *= 2) {
        char * status_string = ad_uds_dtc_status_to_string(i);
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
        ad_list_DTC_DESCRIPTION_free(dtc->description);
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
int ad_list_UDS_DTC_cmp(final ad_list_UDS_DTC * e1, final ad_list_UDS_DTC * e2) {
    return e1 - e2;
}
AD_LIST_SRC(ad_list_UDS_DTC);

static ad_list_ad_list_UDS_DTC * ad_uds_read_dtcs_with_mask(final VehicleIFace * iface, final Vehicle * filter, final AD_UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION sub, final byte StatusMask) {
    ad_list_ad_list_UDS_DTC * result = ad_list_ad_list_UDS_DTC_new();
    viface_lock(iface);
    final Buffer * binRequest = ad_buffer_from_ints(AD_UDS_SERVICE_READ_DTC_INFORMATION, sub);
    if ( sub == AD_UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_DTC_BY_STATUS_MASK ) {
        ad_buffer_append_byte(binRequest, StatusMask);
    }
    viface_send(iface, binRequest);
    ad_buffer_free(binRequest);
    viface_clear_data(iface);
    viface_recv(iface);
    for(int i = 0; i < iface->vehicle->ecus->size; i++) {
        final ad_object_ECU * ecu = iface->vehicle->ecus->list[i];
        final ad_list_UDS_DTC * ecu_response = ad_list_UDS_DTC_new();
        ecu_response->ecu = ecu;
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            final Buffer * data = ecu->data_buffer->list[j];
            if ( data->size < 3 ) {
                log_msg(LOG_WARNING, "received some data with wrong size (probably concurrent access)");
                ad_buffer_dump(data);
                continue;
            }
            if ( data->buffer[0] == UDS_NEGATIVE_RESPONSE ) {
                log_msg(LOG_DEBUG, "negative response found: ");
                ad_buffer_dump(data);
            } else if ( (data->buffer[0] & UDS_POSITIVE_RESPONSE) == UDS_POSITIVE_RESPONSE ) {
                if ( (data->buffer[1] & sub) == sub ) {
                    ecu_response->Status_Availability_Mask = data->buffer[2];
                    for(int i = 3; i < (data->size-DTC_DATA_SZ); i += (DTC_DATA_SZ+1)) {
                        final UDS_DTC * dtc = UDS_DTC_new();
                        memcpy(dtc->data, data->buffer + i, DTC_DATA_SZ);
                        dtc->status = data->buffer[i+DTC_DATA_SZ];
                        dtc->ecu = ecu;
                        ad_list_UDS_DTC_append(ecu_response, dtc);
                        ad_dtc_fetch_from_db((DTC*)dtc, filter);
                    }
                }
            } else {
                log_msg(LOG_WARNING, "Unknown byte at first");
                ad_buffer_dump(data);
            }
        }
        ad_list_ad_list_UDS_DTC_append(result, ecu_response);
    }
    viface_unlock(iface);
    return result;
}
ad_list_ad_list_UDS_DTC * ad_uds_read_dtc_by_status_mask(final VehicleIFace * iface, final Vehicle * filter, final byte StatusMask) {
    return ad_uds_read_dtcs_with_mask(iface, filter, AD_UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_DTC_BY_STATUS_MASK, StatusMask);
}
ad_list_ad_list_UDS_DTC * ad_uds_read_dtc_first_confirmed_dtc(final VehicleIFace * iface, final Vehicle * filter) {
    return ad_uds_read_dtcs_with_mask(iface, filter, AD_UDS_SERVICE_READ_DTC_INFORMATION_SUB_FUNCTION_FIRST_CONFIRMED_DTC, 0x00);
}
ad_list_UDS_DTC * ad_uds_read_all_dtcs(final VehicleIFace * iface, final Vehicle * filter) {
    ad_list_ad_list_UDS_DTC * lists_dtcs = ad_uds_read_dtc_by_status_mask(iface, filter,
        UDS_DTC_STATUS_TestFailed | UDS_DTC_STATUS_TestFailedThisOperationCycle |
        UDS_DTC_STATUS_PendingDTC | UDS_DTC_STATUS_ConfirmedDTC |
        UDS_DTC_STATUS_TestNotCompletedSinceLastClear | UDS_DTC_STATUS_TestFailedSinceLastClear |
        UDS_DTC_STATUS_TestNotCompletedThisOperationCycle | UDS_DTC_STATUS_WarningIndicatorRequested
    );
    ad_list_UDS_DTC * dtcs = ad_list_UDS_DTC_new();
    for(int i = 0; i < lists_dtcs->size; i ++) {
        ad_list_DTC_append_list((ad_list_DTC*)dtcs, (ad_list_DTC*)lists_dtcs->list[i]);
    }
    if ( lists_dtcs->size == 1 ) {
        dtcs->ecu = lists_dtcs->list[0]->ecu;
    }
    return dtcs;
}
void ad_uds_dtc_dump(final UDS_DTC * dtc) {
    log_msg(LOG_DEBUG, "%s", dtc->to_string(AD_DTC(dtc)));
}

char * ad_uds_dtc_status_to_string(UDS_DTC_STATUS wanted) {
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