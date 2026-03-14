#include "libautodiag/model/dtc.h"


AD_LIST_SRC_NEW(DTC_DESCRIPTION,)
void ad_list_DTC_DESCRIPTION_append(ad_list_DTC_DESCRIPTION * list, DTC_DESCRIPTION *desc) {
    list->list = (DTC_DESCRIPTION*)realloc(list->list, sizeof(DTC_DESCRIPTION) * ++list->size);
    list->list[list->size-1] = *desc;
}
void ad_list_DTC_DESCRIPTION_free(ad_list_DTC_DESCRIPTION * list) {
    AD_LIST_FREE_CONTIGUOUS(list);
}

DTC_DESCRIPTION * dtc_description_new() {
    DTC_DESCRIPTION * desc = (DTC_DESCRIPTION*)malloc(sizeof(DTC_DESCRIPTION));
    desc->vehicle = null;
    desc->reason = null;
    desc->solution = null;
    return desc;
}
void dtc_description_free(DTC_DESCRIPTION *desc) {
    if ( desc->vehicle != null ) {
        desc->vehicle = null;
    }
    if ( desc->reason != null ) {
        free(desc->reason);
        desc->reason = null;
    }
    if ( desc->solution != null ) {
        free(desc->solution);
        desc->solution = null;
    }
}

void dtc_description_dump(DTC_DESCRIPTION *desc) {
    log_msg(LOG_DEBUG, "DTC_DESCRIPTION {");
    log_msg(LOG_DEBUG, "    reason: %s", desc->reason);
    log_msg(LOG_DEBUG, "    solution: %s", desc->solution);
    vehicle_dump(desc->vehicle);
    log_msg(LOG_DEBUG, "}");
}

DTC * ad_list_DTC_get(ad_list_DTC * list, char *dtcStr) {
    AD_LIST_FOREACH(list, DTC, dtc, 

        final char * searched_dtc = dtc->to_string((struct DTC *)dtc);
        if ( strcmp(searched_dtc, dtcStr) == 0) {
            return dtc;
        }

    )
    return null;
}

void dtc_description_fetch_from_fs(final DTC * dtc, final Vehicle* filter) {
    final char * basepath = installation_folder_resolve("data/vehicle/");
    log_msg(LOG_ERROR, "TODO");
    free(basepath);
}
int DTC_cmp(DTC* e1, DTC* e2) {
    return memcmp(e1->data, e2->data, DTC_DATA_SZ);
}
void ad_list_DTC_append_list(ad_list_DTC * list, ad_list_DTC * another) {
    if ( another != null ) {
        for(int i = 0; i < another->size; i++) {
            ad_list_DTC_append(list,another->list[i]);
        }
    }
}
AD_LIST_SRC(DTC)