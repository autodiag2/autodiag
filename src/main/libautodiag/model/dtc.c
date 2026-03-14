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
    printf("DTC_DESCRIPTION {\n");
    printf("    reason: %s\n", desc->reason);
    printf("    solution: %s\n", desc->solution);
    vehicle_dump(desc->vehicle);
    printf("}\n");
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

static bool read_tsv_line_dtc(Buffer * line, void*data) {
    void **ptrs = data;
    char * searched_dtc = (char*)ptrs[0];
    DTC_DESCRIPTION *dtc_desc = (DTC_DESCRIPTION *)ptrs[1];

    if ( 0 < line->size ) {
        if ( line->buffer[0] == '#' ) {

        } else {
            char * firstTab = strchr((char *)line->buffer,'\t');
            char * secondTab = null;
            if ( firstTab != null ) {
                *firstTab = 0;
                secondTab = strchr((char *)line->buffer,'\t');
                if ( secondTab != null ) {
                    *secondTab = 0;
                }
            }
            if ( strcmp(searched_dtc,(char*)line->buffer) == 0 ) {
                if ( firstTab != null ) {
                    dtc_desc->reason = strdup(firstTab + 1);
                }
                if ( secondTab != null ) {
                    dtc_desc->solution = strdup(secondTab + 1);
                }
            }
        }
    }

    return true;
}

static bool read_tsv_dtcs(char *fileName, char * searched_dtc, DTC_DESCRIPTION *dtc_desc) {
    void ** ptrs = (void**)malloc(sizeof(void*)*2);
    ptrs[0] = searched_dtc;
    ptrs[1] = dtc_desc;
    final bool res = file_read_lines(fileName,read_tsv_line_dtc,ptrs);
    free(ptrs);
    return res;
}

void dtc_desc_fill_from_codes_file(final DTC * dtc, final DTC_DESCRIPTION * dtc_desc) {
    char *codesFile;
    log_msg(LOG_ERROR, "TODO");
    //asprintf(&codesFile, "%s/codes.tsv", dtc_desc->vehicle->internal.directory);
    final char * searched_dtc = dtc->to_string((struct DTC *)dtc);
    if ( ! read_tsv_dtcs(codesFile,searched_dtc,dtc_desc) ) {
        log_msg(LOG_ERROR, "error while parsing the codes file");
    }
    free(codesFile);
}

static void dtc_description_fetch_from_fs_recurse(final char*path, final DTC * dtc, final Vehicle* filter) {
    log_msg(LOG_ERROR, "TODO");
}

void dtc_description_fetch_from_fs(final DTC * dtc, final Vehicle* filter) {
    final char * basepath = installation_folder_resolve("data/vehicle/");
    dtc_description_fetch_from_fs_recurse(basepath,dtc, filter);
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