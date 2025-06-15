#include "libautodiag/com/obd/saej1979/stored_dtcs.h"

SAEJ1979_DTC * saej1979_dtc_new() {
    SAEJ1979_DTC * dtc = (SAEJ1979_DTC*)malloc(sizeof(SAEJ1979_DTC));
    dtc->description = SAEJ1979_DTC_DESCRIPTION_list_new();
    return dtc;
}
void saej1979_dtc_free(SAEJ1979_DTC *dtc) {
    if ( dtc->description != null ) {
        SAEJ1979_DTC_DESCRIPTION_list_free(dtc->description);
        dtc->description = null;
    }
}

SAEJ1979_DTC * SAEJ1979_DTC_list_get(SAEJ1979_DTC_list * list, char *dtcStr) {
    LIST_FOREACH(list, SAEJ1979_DTC, dtc, 

        final char * searched_dtc = saej1979_dtc_to_string(dtc);
        if ( strcmp(searched_dtc, dtcStr) == 0) {
            return dtc;
        }

    )
    return null;
}

bool saej1979_read_tsv_line(Buffer * line, void*data) {
    void **ptrs = data;
    char * searched_dtc = (char*)ptrs[0];
    SAEJ1979_DTC_DESCRIPTION *dtc_desc = (SAEJ1979_DTC_DESCRIPTION *)ptrs[1];

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

bool saej1979_read_tsv(char *fileName, char * searched_dtc, SAEJ1979_DTC_DESCRIPTION *dtc_desc) {
    void ** ptrs = (void**)malloc(sizeof(void*)*2);
    ptrs[0] = searched_dtc;
    ptrs[1] = dtc_desc;
    final bool res = file_read_lines(fileName,saej1979_read_tsv_line,ptrs);
    free(ptrs);
    return res;
}

void saej1979_fill_dtc_from_codes_file(final SAEJ1979_DTC * dtc, final SAEJ1979_DTC_DESCRIPTION * dtc_desc) {
    char *codesFile;
    asprintf(&codesFile, "%s/codes.tsv", dtc_desc->vehicle->internal.directory);
    final char * searched_dtc = saej1979_dtc_to_string(dtc);
    if ( ! saej1979_read_tsv(codesFile,searched_dtc,dtc_desc) ) {
        log_msg(LOG_ERROR, "error while parsing the codes file");
    }
    free(codesFile);
}

void saej1979_fetch_dtc_description_from_fs_recurse(final char*path, final SAEJ1979_DTC * dtc, final Vehicle* filter) {
    #if defined OS_POSIX
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
                                saej1979_fetch_dtc_description_from_fs_recurse(nextPath,dtc,filter);
                                free(nextPath);
                            }
                            break;
                        }
                        case DT_REG:
                        case DT_BLK:
                        case DT_FIFO:
                        case DT_CHR: {
                            isPathCarDirectory = true;
                            final Vehicle* compare_against = db_vehicle_load_from_directory(path);
                            bool match = false;
                            if ( filter == null ) {
                                match = true;
                            } else {
                                if ( filter->manufacturer == null ) {
                                    match = true;
                                } else if ( strcmp(compare_against->manufacturer, filter->manufacturer) == 0 || strcmp(compare_against->manufacturer, "Generic") == 0 ) {
                                    bool isGeneric = strcmp(compare_against->manufacturer, "Generic") == 0;
                                    if ( isGeneric ) {
                                        match = true;
                                    } else {
                                        if ( filter->engine == null) {
                                            match = true;
                                        } else if ( compare_against->engine != null && strcmp(compare_against->engine, filter->engine) == 0 ) {
                                            match = true;
                                        }
                                    }
                                } 
                            }
                            if ( match ) {
                                final SAEJ1979_DTC_DESCRIPTION * dtc_desc = saej1979_dtc_description_new();
                                dtc_desc->vehicle = compare_against;
                                saej1979_fill_dtc_from_codes_file(dtc, dtc_desc);
                                SAEJ1979_DTC_DESCRIPTION_list_append(dtc->description,dtc_desc);
                            } else {
                                vehicle_free(compare_against);
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
    #else
    #   warning Unsupported OS        
    #endif
}

void saej1979_fetch_dtc_description_from_fs(final SAEJ1979_DTC * dtc, final Vehicle* filter) {
    final char * basepath = installation_folder("data/vehicle/");
    saej1979_fetch_dtc_description_from_fs_recurse(basepath,dtc, filter);
    free(basepath);
}

void SAEJ1979_DTC_list_append_list(SAEJ1979_DTC_list * list, SAEJ1979_DTC_list * another) {
    if ( another != null ) {
        for(int i = 0; i < another->size; i++) {
            SAEJ1979_DTC_list_append(list,another->list[i]);
        }
    }
}

#define saej1979_dtcs_iterator(data) { \
    if ( result == null ) { \
        result = SAEJ1979_DTC_list_new(); \
    } \
    for(int byte = 0; byte < data->size-1; byte += 2) { \
        unsigned char byte_0 = data->buffer[byte]; \
        unsigned char byte_1 = data->buffer[byte+1]; \
        if ( byte_0 == 0 && byte_1 == 0 ) { \
            continue; \
        } else { \
            SAEJ1979_DTC * dtc = saej1979_dtc_new(); \
            dtc->type = (byte_0 & 0xC0) >> 6; \
            sprintf((char*)&(dtc->number),"%x%x%x%x", (byte_0 & 0x30) >> 4, byte_0 & 0xF, (byte_1 & 0xF0) >> 4, byte_1 & 0xF); \
            saej1979_fetch_dtc_description_from_fs(dtc, filter); \
            SAEJ1979_DTC_list_append(result, dtc); \
        } \
    } \
}

SAEJ1979_GENERATE_OBD_REQUEST_ITERATE(
                        SAEJ1979_DTC_list *,saej1979_retrieve_stored_dtcs,
                        "03",saej1979_dtcs_iterator,null,
                        ecu->obd_service.current_dtc, Vehicle *filter
                    )
SAEJ1979_GENERATE_OBD_REQUEST_ITERATE(
                        SAEJ1979_DTC_list *,saej1979_retrieve_pending_dtcs,
                        "07",saej1979_dtcs_iterator,null,
                        ecu->obd_service.pending_dtc, Vehicle *filter
                    )
SAEJ1979_GENERATE_OBD_REQUEST_ITERATE(
                        SAEJ1979_DTC_list *, saej1979_retrieve_permanent_dtcs,
                        "0A",saej1979_dtcs_iterator,null,
                        ecu->obd_service.permanent_dtc, Vehicle *filter
                    )

char * saej1979_dtc_to_string(final SAEJ1979_DTC * dtc) {
    char *result;
    asprintf(&result, "%c%s",iso15031_dtc_type_first_letter(dtc->type),dtc->number);
    return result;
}
Buffer* saej1979_dtc_bin_from_string(char *dtc_string) {
    final Buffer * dtc_bin = buffer_from_ascii_hex(&(dtc_string[1]));
    if ( dtc_bin == null ) {
        return null;
    } else {
        ISO15031_DTC_TYPE dtc_type = iso15031_dtc_first_letter_to_type(dtc_string[0]);
        if ( ISO15031_DTC_TYPE_UNKNOWN == dtc_type ) {
            buffer_free(dtc_bin);
            return null;
        } else {
            dtc_bin->buffer[0] |= dtc_type << 6;
            return dtc_bin;
        }
    }
}

char * saej1979_dtc_categorization_string(final SAEJ1979_DTC * dtc) {
    char * res;
    char * sub = iso15031_dtc_to_subsystem_string(dtc->number);
    char * type = iso15031_dtc_type_to_string(dtc->type);
    asprintf(&res,"Fault on %s, this is a %s DTC\nIt is related to %s\n",
        type, iso15031_dtc_is_generic_code(dtc->number[0]) ? "generic" : "manufacturer specific",
        sub  
    );
    free(sub);
    free(type);
    return res;
}

LIST_DEFINE_MEMBERS_SYM_AUTO(SAEJ1979_DTC)

LIST_NEW_SYM(SAEJ1979_DTC_DESCRIPTION_list)
void SAEJ1979_DTC_DESCRIPTION_list_append(SAEJ1979_DTC_DESCRIPTION_list * list, SAEJ1979_DTC_DESCRIPTION *desc) {
    list->list = (SAEJ1979_DTC_DESCRIPTION*)realloc(list->list, sizeof(SAEJ1979_DTC_DESCRIPTION) * ++list->size);
    list->list[list->size-1] = *desc;
}
void SAEJ1979_DTC_DESCRIPTION_list_free(SAEJ1979_DTC_DESCRIPTION_list * list) {
    LIST_FREE_CONTIGUOUS(list);
}

SAEJ1979_DTC_DESCRIPTION * saej1979_dtc_description_new() {
    SAEJ1979_DTC_DESCRIPTION * desc = (SAEJ1979_DTC_DESCRIPTION*)malloc(sizeof(SAEJ1979_DTC_DESCRIPTION));
    desc->vehicle = null;
    desc->reason = null;
    desc->solution = null;
    return desc;
}
void saej1979_dtc_description_free(SAEJ1979_DTC_DESCRIPTION *desc) {
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

void saej1979_dtc_description_dump(SAEJ1979_DTC_DESCRIPTION *desc) {
    printf("SAEJ1979_DTC_DESCRIPTION {\n");
    printf("    reason: %s\n", desc->reason);
    printf("    solution: %s\n", desc->solution);
    vehicle_dump(desc->vehicle);
    printf("}\n");
}

