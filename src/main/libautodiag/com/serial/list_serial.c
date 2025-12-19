#include "libautodiag/com/serial/list_serial.h"

list_Serial list_serial = { .list = null, .size = 0};
int list_serial_selected = SERIAL_AD_LIST_NO_SELECTED;

Serial * list_serial_get_selected() {
    if ( SERIAL_AD_LIST_NO_SELECTED == list_serial_selected ) {
        return null;
    } else {
        assert(0 <= list_serial_selected && list_serial_selected < list_serial.size);
        return list_serial.list[list_serial_selected];
    }
}

Serial * list_serial_add_if_not_in(Serial * element) {
    for(int i = 0; i < list_serial.size; i++) {
        Serial * serial = list_serial.list[i];
        if ( strcmp(serial->location,element->location) == 0 ) {
            return serial;
        }
    }
    list_Serial_append(&list_serial, element);
    return element;
}

Serial * list_serial_add_if_not_in_by_location(char * location) {
    Serial * serial = list_serial_find_by_location(location);
    if ( serial == null ) {
        list_Serial_append(&list_serial, serial_new());
        final Serial * newOne = list_serial.list[list_serial.size-1];
        serial_set_location(newOne,location);
        return newOne;
    } else {
        return serial;
    }
}

AD_LIST_SRC_APPEND(Serial)

int list_Serial_index_from_location(char *location) {
    if ( location != null ) {
        Serial * port;
        for(int i = 0; i < list_serial.size; i++) {
            port = list_serial.list[i];
            if ( port->location != null && strcmp(port->location,location) == 0 ) {
                return i;
            }
        }
    }
    return -1;
}
void list_serial_set_selected_by_location(char *location) {
    final int index = list_Serial_index_from_location(location);
    if ( index == -1 ) {
        list_serial_selected = SERIAL_AD_LIST_NO_SELECTED;
    } else {
        list_serial_selected = index;
    }
}

Serial * list_serial_find_by_location(final char * location) {
    final int index = list_Serial_index_from_location(location);
    if ( index == -1 ) {
        return null;
    } else {
        return list_serial.list[index];
    }
}

void serial_close_selected() {
    serial_close(list_serial_get_selected());
}

void list_serial_free() {
    if ( list_serial.list != null ) {
        for(int i = 0; i < list_serial.size; i++) {
            if ( list_serial.list[i] != null ) {
                serial_free(list_serial.list[i]);
                list_serial.list[i] = null;
            }
        }
        free(list_serial.list);
        list_serial.list = null;
    }
    list_serial.size = 0;
    list_serial_selected = SERIAL_AD_LIST_NO_SELECTED;
}
#if defined OS_WINDOWS
    void list_serial_fill_comports(char *selected_serial_path, int *baud_rate) {
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

                final Serial * serial = list_serial_add_if_not_in_by_location(formattedPortNameFullPath);
                serial->detected = true;
                if ( list_serial_selected == SERIAL_AD_LIST_NO_SELECTED ) {
                    if ( selected_serial_path != null && strcmp(selected_serial_path,formattedPortNameFullPath) == 0 ) {
                        list_serial_selected = list_serial.size-1;
                        serial->baud_rate = *baud_rate;
                    }
                }
            }
        }
        
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }
    void list_serial_fill_pipes(char *selected_serial_path, int *baud_rate) {
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
                final Serial * serial = list_serial_add_if_not_in_by_location(pipeFullPath);
                serial->detected = true;
                if ( list_serial_selected == SERIAL_AD_LIST_NO_SELECTED ) {
                    if ( selected_serial_path != null && strcmp(selected_serial_path,pipeFullPath) == 0 ) {
                        list_serial_selected = list_serial.size-1;
                        serial->baud_rate = *baud_rate;
                    }
                }
            }
        } while (FindNextFileA(hFind, &findFileData));

        FindClose(hFind);
    }
#elif defined OS_POSIX
    void list_serial_fill_from_dir(final char * dir, int filter_sz, char filter[][20], char * selected_serial_path, int * baud_rate) {
        
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
                                final Serial * serial = list_serial_add_if_not_in_by_location(serial_path);
                                serial->detected = true;
                                if ( access(serial_path,R_OK|W_OK) == 0 ) {
                                    log_msg(LOG_DEBUG, "%s: All permissions granted", serial_path);
                                } else {
                                    log_msg(LOG_DEBUG, "%s: Missing permissions", serial_path);
                                }
                                if ( list_serial_selected == SERIAL_AD_LIST_NO_SELECTED ) {
                                    if ( selected_serial_path != null && strcmp(selected_serial_path,serial_path) == 0 ) {
                                        list_serial_selected = list_serial.size-1;
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

void list_serial_set_to_undetected() {
    for(int i = 0; i < list_serial.size; i++) {
        Serial * serial = list_serial.list[i];
        serial->detected = false;
    }
}
bool list_serial_remove(final Serial * element) {
    int index = -1;
    for(int i = 0; i < list_serial.size; i++) {
        if ( list_serial.list[i] == element ) {
            index = i;
            break;
        }
    }
    if ( index == -1 ) {
        return false;
    } else {
        if ( index < list_serial_selected ) {
            list_serial_selected --;
        }
        for ( int i = index; i < list_serial.size-1; i++) {
            list_serial.list[i] = list_serial.list[i+1];
        }
        list_serial.list = (Serial **)realloc(list_serial.list, (--list_serial.size) * sizeof(Serial*));
        serial_free(element);
        return true;
    }
}
void list_serial_remove_undetected() {
    for(int i = 0; i < list_serial.size; i++) {
        Serial * serial = list_serial.list[i];
        if ( ! serial->detected ) {
            if ( i == list_serial_selected ) {
                list_serial_selected = SERIAL_AD_LIST_NO_SELECTED;
            }
            list_serial_remove(serial);
        }
    }
}

void list_serial_fill() {
    module_debug(MODULE_SERIAL "Filling serial list (with update take care)");
    int i;
    char * selected_serial_path = null;
    int baud_rate = SERIAL_DEFAULT_BAUD_RATE;
    final Serial * selected_serial = list_serial_get_selected();
    if ( selected_serial != null ) {
        selected_serial_path = strdup(selected_serial->location);
        baud_rate = selected_serial->baud_rate;
    }
    list_serial_set_to_undetected();
   
    #if defined OS_WINDOWS
        list_serial_fill_comports(selected_serial_path,&baud_rate);
        list_serial_fill_pipes(selected_serial_path, &baud_rate);
    #elif defined OS_POSIX 
		char part1[][20] = {"ttys","ttyS","ttyUSB"};
	    list_serial_fill_from_dir("/dev/",3,part1,selected_serial_path,&baud_rate);
		char part2[][20] = {""};
		list_serial_fill_from_dir("/dev/pts/",1,part2,selected_serial_path,&baud_rate);
    #else
    #   warning Unsupported OS
    #endif

    list_serial_remove_undetected();
    if ( selected_serial_path != null ) {
        free(selected_serial_path);
    }

}
