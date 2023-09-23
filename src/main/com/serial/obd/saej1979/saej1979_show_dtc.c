#include "com/serial/obd/saej1979/saej1979_show_dtc.h"

SAEJ1979_DTC * saej1979_dtc_new() {
    SAEJ1979_DTC * dtc = (SAEJ1979_DTC*)malloc(sizeof(SAEJ1979_DTC));
    dtc->description = saej1979_dtc_description_list_new();
    return dtc;
}
void saej1979_dtc_free(SAEJ1979_DTC *dtc) {
    if ( dtc->description != null ) {
        saej1979_dtc_description_list_free(dtc->description);
        dtc->description = null;
    }
}

SAEJ1979_DTC * saej1979_dtc_list_get(SAEJ1979_DTC_list * list, char *dtcStr) {
    for(int i = 0; i < list->size; i++) {
        SAEJ1979_DTC *dtc = list->list[i];
        final char * searched_dtc = saej1979_dtc_to_string(dtc);
        if ( strcmp(searched_dtc, dtcStr) == 0) {
            return dtc;
        }
    }
    return null;
}

void saej1979_fill_dtc_from_codes_file(final SAEJ1979_DTC * dtc, final SAEJ1979_DTC_DESCRIPTION * dtc_desc) {
    char *codesFile;
    asprintf(&codesFile, "%s/codes.tsv", dtc_desc->car->internal.directory);
    FILE * codesFD = fopen(codesFile, "r");
    if ( codesFD == null ) {
        perror("fopen");
    } else {
        final char * searched_dtc = saej1979_dtc_to_string(dtc);
        char *line = null;
        size_t len = 0;
        ssize_t nread;
        char *firstTab, *secondTab;
                   
        while ((nread = getline(&line, &len, codesFD)) != -1) {
            if ( line[nread-1] == '\n' ) {
                line[nread-1] = 0;
            }
            firstTab = strchr(line,'\t');
            secondTab = null;
            if ( firstTab != null ) {
                *firstTab = 0;
                secondTab = strchr(line,'\t');
                if ( secondTab != null ) {
                    *secondTab = 0;
                }
            }
            if ( strcmp(searched_dtc,line) == 0 ) {
                if ( firstTab != null ) {
                    dtc_desc->reason = firstTab + 1;
                }
                if ( secondTab != null ) {
                    dtc_desc->solution = secondTab + 1;
                }
            } else {
                free(line);
            }
            line = null;
            len = 0;
        }
        fclose(codesFD);
    }
    free(codesFile);
}

void saej1979_fetch_dtc_description_from_fs_recurse(final char*path, final SAEJ1979_DTC * dtc) {
    struct dirent **namelist;
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
                            saej1979_fetch_dtc_description_from_fs_recurse(nextPath,dtc);
                            free(nextPath);
                        }
                        break;
                    }
                    case DT_REG:
                    case DT_BLK:
                    case DT_FIFO:
                    case DT_CHR: {
                        final SAEJ1979_DTC_DESCRIPTION * dtc_desc = saej1979_dtc_description_new();
                        dtc_desc->car = car_model_load_from_directory(path);
                        saej1979_fill_dtc_from_codes_file(dtc, dtc_desc);
                        saej1979_dtc_description_list_append(dtc->description,dtc_desc);
                        isPathCarDirectory = true;
                        break;
                    }
                    default: {
                        char *msg;
                        asprintf(&msg, "Unknown file type=%d(%s)", namelist[namelist_n]->d_type, namelist[namelist_n]->d_name);
                        log_msg(msg, LOG_DEBUG);
                        free(msg);
                    }   
                }
            }
            free(namelist[namelist_n]);
        }
    }
}

void saej1979_fetch_dtc_description_from_fs(final SAEJ1979_DTC * dtc) {
    char * basepath;
    final char * path = config_get_data_directory_safe();
    asprintf(&basepath, "%s/data/car/", path);
    free(path);
    saej1979_fetch_dtc_description_from_fs_recurse(basepath,dtc);
    free(basepath);
}

void saej1979_dtc_list_append_list(SAEJ1979_DTC_list * list, SAEJ1979_DTC_list * another) {
    if ( another != null ) {
        for(int i = 0; i < another->size; i++) {
            saej1979_dtc_list_append(list,another->list[i]);
        }
    }
}

void* saej1979_retrieve_dtcs_handler(final OBDIFACE iface, final int response, void *arg) {
    SAEJ1979_DTC_list *list;
    if ( response < 0 ) {
        log_msg("Error during reception", LOG_ERROR);
        list = null;
    } else {
        list = saej1979_dtc_list_new();
        for( int i = 0; i < iface->obd_data_buffer->size; i++) {
            BUFFER data = iface->obd_data_buffer->list[i];
            for(int byte = 0; byte < data->size_used-1; byte += 2) {
                unsigned char byte_0 = data->buffer[byte];
                unsigned char byte_1 = data->buffer[byte+1];
                if ( byte_0 == 0 && byte_1 == 0 ) {
                    continue;
                } else {
                    SAEJ1979_DTC * dtc = saej1979_dtc_new();
                    dtc->type = (byte_0 & 0xC0) >> 6;
                    sprintf((char*)&(dtc->number),"%x%x%x%x", (byte_0 & 0x30) >> 4, byte_0 & 0xF, (byte_1 & 0xF0) >> 4, byte_1 & 0xF);
                    saej1979_fetch_dtc_description_from_fs(dtc);
                    saej1979_dtc_list_append(list, dtc);
                }
            }
        }
    }
    return (void*)list;
}

SAEJ1979_DTC_list * saej1979_retrieve_dtcs(final OBDIFACE iface, final char *service) {
    return (SAEJ1979_DTC_list*)obd_request(iface,saej1979_retrieve_dtcs_handler,null,service,false);
}
SAEJ1979_DTC_list * saej1979_retrieve_stored_dtcs(final OBDIFACE iface) {
    return saej1979_retrieve_dtcs(iface,"03");
}
SAEJ1979_DTC_list * saej1979_retrieve_pending_dtcs(final OBDIFACE iface) {
    return saej1979_retrieve_dtcs(iface,"07");
}
SAEJ1979_DTC_list * saej1979_retrieve_permanent_dtcs(final OBDIFACE iface) {
    return saej1979_retrieve_dtcs(iface,"0A");
}
char * saej1979_dtc_type_to_string(final SAEJ1979_DTC_TYPE type) {
    switch(type) {
        case SAEJ1979_DTC_TYPE_POWERTRAIN:
            return strdup("power train");
        case SAEJ1979_DTC_TYPE_CHASSIS:
            return strdup("chassis");
        case SAEJ1979_DTC_TYPE_BODY:
            return strdup("body");
        case SAEJ1979_DTC_TYPE_NETWORK:
            return strdup("network");
    }
    return strdup("unknown");
}

char * saej1979_dtc_to_string(final SAEJ1979_DTC * dtc) {
    char value = '?';
    char *result;
    switch(dtc->type) {
        case SAEJ1979_DTC_TYPE_POWERTRAIN:
            value = 'P';
            break;
        case SAEJ1979_DTC_TYPE_CHASSIS:
            value = 'C';
            break;
        case SAEJ1979_DTC_TYPE_BODY:
            value = 'B';
            break;
        case SAEJ1979_DTC_TYPE_NETWORK:
            value = 'U';
            break;
    }
    asprintf(&result, "%c%s",value,dtc->number);
    return result;
}

bool saej1979_dtc_is_generic_code(final SAEJ1979_DTC * dtc) {
    switch(dtc->number[0]) {
        case '0':
        case '2':
        case '3': 
            return true;
    }
    return false;
}
char * saej1979_dtc_to_subsystem_string(final SAEJ1979_DTC * dtc) {
    if ( saej1979_dtc_is_generic_code(dtc) ) {
        switch(dtc->number[1]) {
            case '1': return strdup("fuel & air metering");
            case '2': return strdup("fuel & air metering (injector circuit)");
            case '3': return strdup("ignition system or misfire");
            case '4': return strdup("auxiliary emission controls");
            case '5': return strdup("vehicle speed control & idle control system");
            case '6': return strdup("computer cutput circuit");
            case '7':
            case '8':
            case '9':
                return strdup("transmission (gearbox)");
            case 'A':
            case 'B':
            case 'C':
                return strdup("hybrid propulsion");
        }
        return strdup("unknown");
    } else {
        return strdup("manufacturer specific");
    }
}

char * saej1979_dtc_categorization_string(final SAEJ1979_DTC * dtc) {
    char * res;
    char * sub = saej1979_dtc_to_subsystem_string(dtc);
    char * type = saej1979_dtc_type_to_string(dtc->type);
    asprintf(&res,"Fault on %s, this is a %s DTC\nIt is related to %s\n",
        type, saej1979_dtc_is_generic_code(dtc) ? "generic" : "manufacturer specific",
        sub  
    );
    free(sub);
    free(type);
    return res;
}

SAEJ1979_DTC_list * saej1979_dtc_list_new() {
    SAEJ1979_DTC_list * list = (SAEJ1979_DTC_list*)malloc(sizeof(SAEJ1979_DTC_list));
    LIST_NEW(list);
    return list;
}

void saej1979_dtc_list_free(SAEJ1979_DTC_list * list) {
    LIST_FREE(list);
}
void saej1979_dtc_list_append(SAEJ1979_DTC_list * list, SAEJ1979_DTC *dtc) {
    list->size++;
    list->list = (SAEJ1979_DTC**)realloc(list->list, sizeof(SAEJ1979_DTC*) * list->size);
    list->list[list->size-1] = dtc;
}
SAEJ1979_DTC_DESCRIPTION_list * saej1979_dtc_description_list_new() {
    SAEJ1979_DTC_DESCRIPTION_list * list = (SAEJ1979_DTC_DESCRIPTION_list*)malloc(sizeof(SAEJ1979_DTC_DESCRIPTION_list));
    list->size = 0;
    list->list = null;
    return list;
}
void saej1979_dtc_description_list_free(SAEJ1979_DTC_DESCRIPTION_list * list) {
    LIST_FREE_CONTIGUOUS(list);
}
void saej1979_dtc_description_list_append(SAEJ1979_DTC_DESCRIPTION_list * list, SAEJ1979_DTC_DESCRIPTION *desc) {
    list->size++;
    list->list = (SAEJ1979_DTC_DESCRIPTION*)realloc(list->list, sizeof(SAEJ1979_DTC_DESCRIPTION) * list->size);
    list->list[list->size-1] = *desc;
}
SAEJ1979_DTC_DESCRIPTION * saej1979_dtc_description_new() {
    SAEJ1979_DTC_DESCRIPTION * desc = (SAEJ1979_DTC_DESCRIPTION*)malloc(sizeof(SAEJ1979_DTC_DESCRIPTION));
    desc->car = null;
    desc->reason = null;
    desc->solution = null;
    return desc;
}
void saej1979_dtc_description_free(SAEJ1979_DTC_DESCRIPTION *desc) {
    if ( desc->car != null ) {
        car_model_free(desc->car);
        free(desc->car);
        desc->car = null;
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
    car_model_dump(desc->car);
    printf("}\n");
}

