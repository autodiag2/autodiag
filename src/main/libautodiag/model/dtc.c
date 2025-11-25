#include "libautodiag/model/dtc.h"


AD_LIST_SRC_NEW(DTC_DESCRIPTION,)
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


DTC * list_DTC_get(list_DTC * list, char *dtcStr) {
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
            char * firstTab = strchr(line->buffer,'\t');
            char * secondTab = null;
            if ( firstTab != null ) {
                *firstTab = 0;
                secondTab = strchr(line->buffer,'\t');
                if ( secondTab != null ) {
                    *secondTab = 0;
                }
            }
            if ( strcmp(searched_dtc,line->buffer) == 0 ) {
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
    asprintf(&codesFile, "%s/codes.tsv", dtc_desc->vehicle->internal.directory);
    final char * searched_dtc = dtc->to_string((struct DTC *)dtc);
    if ( ! read_tsv_dtcs(codesFile,searched_dtc,dtc_desc) ) {
        log_msg(LOG_ERROR, "error while parsing the codes file");
    }
    free(codesFile);
}

static void dtc_description_fetch_from_fs_recurse(final char*path, final DTC * dtc, final Vehicle* filter) {
    DIRENT **namelist;   
    final int namelist_n = scandir(path, &namelist,NULL,&alphasort);

    if ( namelist_n == -1 ) {
        char *path_str;
        asprintf(&path_str,"scandir error for %s\n", path);
        perror(path_str);
        free(path_str);
    } else {
        bool isPathCarDirectory = false;
        while (namelist_n--) {
            if ( ! isPathCarDirectory ) {
                switch(namelist[namelist_n]->d_type) {
                    case DT_DIR: {
                        if ( strcmp(namelist[namelist_n]->d_name, ".") != 0 
                        && strcmp(namelist[namelist_n]->d_name, "..") != 0 ) {
                            char *nextPath;
                            asprintf(&nextPath, "%s/%s", path, namelist[namelist_n]->d_name);
                            dtc_description_fetch_from_fs_recurse(nextPath,dtc,filter);
                            free(nextPath);
                        }
                        break;
                    }
                    case DT_REG:
                    case DT_BLK:
                    case DT_FIFO:
                    case DT_CHR: {
                        if ( strcmp(namelist[namelist_n]->d_name, "desc.ini") == 0 ) {
                            isPathCarDirectory = true;
                            final Vehicle* compare_against = db_vehicle_load_from_directory(path);
                            bool match = false;
                            if ( filter == null ) {
                                match = true;
                            } else {
                                if ( filter->manufacturer == null || strlen(filter->manufacturer) == 0 ) {
                                    match = true;
                                } else if ( strcmp(compare_against->manufacturer, filter->manufacturer) == 0 || strcmp(compare_against->manufacturer, "Generic") == 0 ) {
                                    bool isGeneric = strcmp(compare_against->manufacturer, "Generic") == 0;
                                    if ( isGeneric ) {
                                        match = true;
                                    } else {
                                        if ( filter->engine == null || strlen(filter->engine) == 0 ) {
                                            match = true;
                                        } else if ( compare_against->engine != null && strcmp(compare_against->engine, filter->engine) == 0 ) {
                                            match = true;
                                        }
                                    }
                                } 
                            }
                            if ( match ) {
                                final DTC_DESCRIPTION * dtc_desc = dtc_description_new();
                                dtc_desc->vehicle = compare_against;
                                dtc_desc_fill_from_codes_file(dtc, dtc_desc);
                                list_DTC_DESCRIPTION_append(dtc->description,dtc_desc);
                            }
                        }
                        break;
                    }
                    default: {
                        log_msg(LOG_DEBUG, "Unknown file type=%d(%s)", namelist[namelist_n]->d_type, namelist[namelist_n]->d_name);
                    }   
                }
            }
            free(namelist[namelist_n]);
        }
    }
}

void dtc_description_fetch_from_fs(final DTC * dtc, final Vehicle* filter) {
    final char * basepath = installation_folder_resolve("data/vehicle/");
    dtc_description_fetch_from_fs_recurse(basepath,dtc, filter);
    free(basepath);
}
int DTC_cmp(DTC* e1, DTC* e2) {
    return memcmp(e1->data, e2->data, DTC_DATA_SZ);
}
void list_DTC_append_list(list_DTC * list, list_DTC * another) {
    if ( another != null ) {
        for(int i = 0; i < another->size; i++) {
            list_DTC_append(list,another->list[i]);
        }
    }
}
AD_LIST_SRC(DTC)