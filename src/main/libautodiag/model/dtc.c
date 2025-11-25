#include "libautodiag/model/dtc.h"


AD_LIST_SRC_NEW(DTC_DESCRIPTION)
void list_DTC_DESCRIPTION_append(list_DTC_DESCRIPTION * list, DTC_DESCRIPTION *desc) {
    list->list = (DTC_DESCRIPTION*)realloc(list->list, sizeof(DTC_DESCRIPTION) * ++list->size);
    list->list[list->size-1] = *desc;
}
void list_DTC_DESCRIPTION_free(list_DTC_DESCRIPTION * list) {
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
    printf("DTC_DESCRIPTION {\n");
    printf("    reason: %s\n", desc->reason);
    printf("    solution: %s\n", desc->solution);
    vehicle_dump(desc->vehicle);
    printf("}\n");
}