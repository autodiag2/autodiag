#include "libautodiag/com/serial/list_serial.h"

int Serial_cmp(Serial * serial1, Serial * serial2) {
    return strcmp(serial1->location, serial2->location);
}
AD_LIST_SRC(Serial)

object_SerialTable * object_SerialTable_new() {
    object_SerialTable * table = malloc(sizeof(object_SerialTable));
    table->list = list_Serial_new();
    table->selected_index = SERIAL_TABLE_NO_SELECTED;
    return table;
}
void object_SerialTable_free(object_SerialTable * table) {
    serial_table_free(table);
    free(table);
}
object_SerialTable * object_SerialTable_assign(object_SerialTable * dest, object_SerialTable * src) {
    log_msg(LOG_ERROR, "Not implemented yet");
    return dest;
}
Serial * serial_table_get_selected(object_SerialTable * table) {
    if ( SERIAL_TABLE_NO_SELECTED == table->selected_index ) {
        return null;
    } else {
        assert(0 <= table->selected_index && table->selected_index < table->list->size);
        return table->list->list[table->selected_index];
    }
}

Serial * serial_table_add_if_not_in(object_SerialTable * table, Serial * element) {
    for(int i = 0; i < table->list->size; i++) {
        Serial * serial = table->list->list[i];
        if ( strcmp(serial->location,element->location) == 0 ) {
            return serial;
        }
    }
    list_Serial_append(table->list, element);
    return element;
}

Serial * serial_table_add_if_not_in_by_location(object_SerialTable * table, char * location) {
    assert(location != null);
    Serial * serial = serial_table_find_by_location(table, location);
    if ( serial == null ) {
        list_Serial_append(table->list, serial_new());
        final Serial * newOne = table->list->list[table->list->size-1];
        device_location_set(AD_DEVICE(newOne),location);
        return newOne;
    } else {
        return serial;
    }
}
bool serial_table_update_device(object_SerialTable * table, Device * old, Device * new) {
    int index = list_Serial_index_of(table->list, old);
    if ( index < 0 ) {
        return false;
    }
    table->list->list[index] = new;
    return true;
}
int serial_table_index_from_location(object_SerialTable * table, char *location) {
    if ( location != null ) {
        Device * device;
        for(int i = 0; i < table->list->size; i++) {
            device = (Device*)table->list->list[i];
            if ( device->location != null && strcmp(device->location,location) == 0 ) {
                return i;
            }
        }
    }
    return -1;
}
void serial_table_set_selected_by_location(object_SerialTable * table, char *location) {
    final int index = serial_table_index_from_location(table, location);
    if ( index == -1 ) {
        table->selected_index = SERIAL_TABLE_NO_SELECTED;
    } else {
        table->selected_index = index;
    }
}

Serial * serial_table_find_by_location(object_SerialTable * table, final char * location) {
    final int index = serial_table_index_from_location(table, location);
    if ( index == -1 ) {
        return null;
    } else {
        return table->list->list[index];
    }
}

void serial_table_close_selected(object_SerialTable * table) {
    serial_close(serial_table_get_selected(table));
}

void serial_table_free(object_SerialTable * table) {
    if ( table->list->list != null ) {
        for(int i = 0; i < table->list->size; i++) {
            if ( table->list->list[i] != null ) {
                serial_free(table->list->list[i]);
                table->list->list[i] = null;
            }
        }
        free(table->list->list);
        table->list->list = null;
    }
    table->list->size = 0;
    table->selected_index = SERIAL_TABLE_NO_SELECTED;
}
#if defined OS_WINDOWS
    static void serial_table_fill_comports(object_SerialTable * table, char *selected_serial_path, int *baud_rate) {
        HDEVINFO hDevInfo;
        SP_DEVINFO_DATA devInfoData;
        DWORD i;
        char portName[256];
        char formattedPortName[256];
        char formattedPortNameFullPath[256];

        hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
        if (hDevInfo == INVALID_HANDLE_VALUE) {
            log_msg(LOG_ERROR, "Error getting device list.\n");
            return;
        }

        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); i++) {
            if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devInfoData, SPDRP_FRIENDLYNAME, NULL, (BYTE*)portName, sizeof(portName), NULL)) {
                
                sscanf(portName, "%*[^(](%[^)])", formattedPortName);
                snprintf(formattedPortNameFullPath, sizeof(formattedPortNameFullPath), "\\\\.\\%s", formattedPortName);

                final Serial * serial = serial_table_add_if_not_in_by_location(table,formattedPortNameFullPath);
                serial->detected = true;
                if ( table->selected_index == SERIAL_TABLE_NO_SELECTED ) {
                    if ( selected_serial_path != null && strcmp(selected_serial_path,formattedPortNameFullPath) == 0 ) {
                        table->selected_index = table->list->size-1;
                        serial->baud_rate = *baud_rate;
                    }
                }
            }
        }
        
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }
    static void serial_table_fill_pipes(object_SerialTable * table, char *selected_serial_path, int *baud_rate) {
        char pipeName[256];
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind;

        log_msg(LOG_INFO, "Liste des pipes existants:");
        
        snprintf(pipeName, sizeof(pipeName), R"(\\.\pipe\*)"); // Match all pipes
        hFind = FindFirstFileA(pipeName, &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            log_msg(LOG_INFO, "Aucun pipe trouvé.");
            return;
        }

        do {
            if (strncmp(findFileData.cFileName, SERIAL_AD_LIST_PIPE_PREFIX, strlen(SERIAL_AD_LIST_PIPE_PREFIX)) == 0) {
                char *pipeFullPath;
                asprintf(&pipeFullPath, "\\\\.\\pipe\\%s", findFileData.cFileName);
                final Serial * serial = serial_table_add_if_not_in_by_location(table,pipeFullPath);
                serial->detected = true;
                if ( table->selected_index == SERIAL_TABLE_NO_SELECTED ) {
                    if ( selected_serial_path != null && strcmp(selected_serial_path,pipeFullPath) == 0 ) {
                        table->selected_index = table->list->size-1;
                        serial->baud_rate = *baud_rate;
                    }
                }
            }
        } while (FindNextFileA(hFind, &findFileData));

        FindClose(hFind);
    }
#elif defined OS_POSIX
    static void serial_table_fill_from_dir(object_SerialTable * table, final char * dir, int filter_sz, char filter[][20], char * selected_serial_path, int * baud_rate) {
        
        DIRENT **namelist;
        final int namelist_n = scandir(dir, &namelist,NULL,&alphasort);
        if ( namelist_n == -1 ) {
            if ( errno != ENOENT ) {
                char *path_str;
                asprintf(&path_str,"scandir error for %s\n", dir);
                perror(path_str);
                free(path_str);
            }
        } else {
            while (namelist_n--) {
                switch(namelist[namelist_n]->d_type) {
                    case DT_DIR: 
                        break;
                    case DT_REG:
                    case DT_BLK:
                    case DT_FIFO:
                    case DT_CHR: {
                        for(int i = 0; i < filter_sz; i++) {
                            if ( filter[i] == null || strncmp(filter[i],namelist[namelist_n]->d_name,strlen(filter[i])) == 0 ) {
                                char *serial_path ;
                                assert(0 < strlen(dir));
                                asprintf(&serial_path,"%s%s%s",dir,dir[strlen(dir)-1] == '/' ? "" : "/",namelist[namelist_n]->d_name);
                                final Serial * serial = serial_table_add_if_not_in_by_location(table, serial_path);
                                serial->detected = true;
                                if ( access(serial_path,R_OK|W_OK) == 0 ) {
                                    log_msg(LOG_DEBUG, "%s: All permissions granted", serial_path);
                                } else {
                                    log_msg(LOG_DEBUG, "%s: Missing permissions", serial_path);
                                }
                                if ( table->selected_index == SERIAL_TABLE_NO_SELECTED ) {
                                    if ( selected_serial_path != null && strcmp(selected_serial_path,serial_path) == 0 ) {
                                        table->selected_index = table->list->size-1;
                                        serial->baud_rate = *baud_rate;
                                    }
                                }
                                free(serial_path);
                            }
                        }
                        break;
                    }
                    default: {
                        log_msg(LOG_DEBUG, "Unknown file type=%d(%s)", namelist[namelist_n]->d_type, namelist[namelist_n]->d_name);
                    }   
                }
                free(namelist[namelist_n]);
                namelist[namelist_n] = null;
            }
            free(namelist);
        }
    }
#endif

void serial_table_set_to_undetected(object_SerialTable * table) {
    for(int i = 0; i < table->list->size; i++) {
        Serial * serial = table->list->list[i];
        serial->detected = false;
    }
}
bool serial_table_remove(object_SerialTable * table, final Serial * element) {
    int index = -1;
    for(int i = 0; i < table->list->size; i++) {
        if ( table->list->list[i] == element ) {
            index = i;
            break;
        }
    }
    if ( index == -1 ) {
        return false;
    } else {
        if ( index < table->selected_index ) {
            table->selected_index --;
        }
        for ( int i = index; i < table->list->size-1; i++) {
            table->list->list[i] = table->list->list[i+1];
        }
        table->list->size--;
        serial_free(element);
        return true;
    }
}
void serial_table_remove_undetected(object_SerialTable * table, bool except_network) {
    for(int i = 0; i < table->list->size; i++) {
        Serial * serial = table->list->list[i];
        if ( ! serial->detected ) {
            if ( except_network && strstr(serial->location,":") != null ) {
                continue;
            }
            if ( i == table->selected_index ) {
                table->selected_index = SERIAL_TABLE_NO_SELECTED;
            }
            serial_table_remove(table, serial);
        }
    }
}

void serial_table_fill(object_SerialTable * table) {
    module_debug(MODULE_SERIAL "Filling serial list (with update take care)");
    int i;
    char * selected_serial_path = null;
    int baud_rate = SERIAL_DEFAULT_BAUD_RATE;
    final Serial * selected_serial = serial_table_get_selected(table);
    if ( selected_serial != null ) {
        selected_serial_path = strdup(selected_serial->location);
        baud_rate = selected_serial->baud_rate;
    }
    serial_table_set_to_undetected(table);
   
    #if defined OS_WINDOWS
        serial_table_fill_comports(table, selected_serial_path,&baud_rate);
        serial_table_fill_pipes(table, selected_serial_path, &baud_rate);
    #elif defined OS_POSIX 
		char part1[][20] = {"ttys","ttyS","ttyUSB"};
	    serial_table_fill_from_dir(table, "/dev/",3,part1,selected_serial_path,&baud_rate);
		char part2[][20] = {""};
		serial_table_fill_from_dir(table, "/dev/pts/",1,part2,selected_serial_path,&baud_rate);
    #else
    #   warning Unsupported OS
    #endif

    // This may free the device after it has been openned so disabling for now
    // So function like thread_viface_cleanup_routine use the device after it has been removed/free by this list
    //serial_table_remove_undetected(table, true);

    if ( selected_serial_path != null ) {
        free(selected_serial_path);
    }

}
