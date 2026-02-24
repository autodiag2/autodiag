#include "libautodiag/com/device_table.h"
#include "libautodiag/com/doip/device.h"
#include "libautodiag/com/serial/serial.h"

int Device_cmp(Device * d1, Device * d2) {
    return strcmp(d1->location, d2->location);
}
AD_LIST_SRC(Device)

object_DeviceTable * object_DeviceTable_new() {
    object_DeviceTable * table = malloc(sizeof(object_DeviceTable));
    table->list = list_Device_new();
    table->selected_index = DEVICE_TABLE_NO_SELECTED;
    return table;
}
void object_DeviceTable_free(object_DeviceTable * table) {
    device_table_free(table);
    free(table);
}
object_DeviceTable * object_DeviceTable_assign(object_DeviceTable * dest, object_DeviceTable * src) {
    log_msg(LOG_ERROR, "Not implemented yet");
    return dest;
}
Device * device_table_get_selected(object_DeviceTable * table) {
    if ( DEVICE_TABLE_NO_SELECTED == table->selected_index ) {
        return null;
    } else {
        assert(0 <= table->selected_index && table->selected_index < table->list->size);
        return table->list->list[table->selected_index];
    }
}

Device * device_table_add_if_not_in(object_DeviceTable * table, Device * element) {
    for(int i = 0; i < table->list->size; i++) {
        Device * device = table->list->list[i];
        if ( strcmp(device->location,element->location) == 0 ) {
            return device;
        }
    }
    list_Device_append(table->list, element);
    return element;
}

Device * device_table_add_if_not_in_by_location(object_DeviceTable * table, char * location, AD_DEVICE_TYPE type) {
    assert(location != null);
    Device * device = device_table_find_by_location(table, location);
    if ( device == null ) {
        switch(type) {
            case AD_DEVICE_TYPE_SERIAL: {
                list_Device_append(table->list, AD_DEVICE(serial_new()));
            } break;
            case AD_DEVICE_TYPE_DOIP: {
                list_Device_append(table->list, AD_DEVICE(object_DoIPDevice_new()));
            } break;
            default: {
                log_msg(LOG_DEBUG, "not implemented");
            } break;
        }
        final Device * newOne = table->list->list[table->list->size-1];
        device_location_set(AD_DEVICE(newOne),location);
        return newOne;
    } else {
        return device;
    }
}
bool device_table_update_device(object_DeviceTable * table, Device * old, Device * new) {
    int index = list_Device_index_of(table->list, old);
    if ( index < 0 ) {
        return false;
    }
    table->list->list[index] = new;
    return true;
}
int device_table_index_from_location(object_DeviceTable * table, char *location) {
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
void device_table_set_selected_by_location(object_DeviceTable * table, char *location) {
    final int index = device_table_index_from_location(table, location);
    if ( index == -1 ) {
        table->selected_index = DEVICE_TABLE_NO_SELECTED;
    } else {
        table->selected_index = index;
    }
}

Device * device_table_find_by_location(object_DeviceTable * table, final char * location) {
    final int index = device_table_index_from_location(table, location);
    if ( index == -1 ) {
        return null;
    } else {
        return table->list->list[index];
    }
}

void device_table_close_selected(object_DeviceTable * table) {
    Device * device = device_table_get_selected(table);
    device->close(device);
}

void device_table_free(object_DeviceTable * table) {
    if ( table->list->list != null ) {
        for(int i = 0; i < table->list->size; i++) {
            if ( table->list->list[i] != null ) {
                Device * device = table->list->list[i];
                if ( device->free == null ) {
                    log_msg(LOG_WARNING, "Cannot free device because operator not set");
                } else {
                    device->free(device);
                }
                table->list->list[i] = null;
            }
        }
        free(table->list->list);
        table->list->list = null;
    }
    table->list->size = 0;
    table->selected_index = DEVICE_TABLE_NO_SELECTED;
}
#if defined OS_WINDOWS
    static void device_table_fill_comports(object_DeviceTable * table, char *selected_device_path, int *baud_rate) {
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

                final Device * device = device_table_add_if_not_in_by_location(table, formattedPortNameFullPath, AD_DEVICE_TYPE_SERIAL);
                if ( device->type == AD_DEVICE_TYPE_SERIAL ) {
                    Serial * serial = (Serial*)device;
                    serial->detected = true;
                    if ( table->selected_index == DEVICE_TABLE_NO_SELECTED ) {
                        if ( selected_device_path != null && strcmp(selected_device_path,formattedPortNameFullPath) == 0 ) {
                            table->selected_index = table->list->size-1;
                            serial->baud_rate = *baud_rate;
                        }
                    }
                } else {
                    log_msg(LOG_ERROR, "Not implemented");
                }
            }
        }
        
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }
    static void device_table_fill_pipes(object_DeviceTable * table, char *selected_device_path, int *baud_rate) {
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
            if (strncmp(findFileData.cFileName, WINDOWS_SERIAL_EMU_PIPE_PREFIX, strlen(WINDOWS_SERIAL_EMU_PIPE_PREFIX)) == 0) {
                char *pipeFullPath;
                asprintf(&pipeFullPath, "\\\\.\\pipe\\%s", findFileData.cFileName);
                final Device * device = device_table_add_if_not_in_by_location(table, pipeFullPath, AD_DEVICE_TYPE_SERIAL);
                if ( device->type == AD_DEVICE_TYPE_SERIAL ) {
                    Serial * serial = (Serial*)device;
                    serial->detected = true;
                    if ( table->selected_index == DEVICE_TABLE_NO_SELECTED ) {
                        if ( selected_device_path != null && strcmp(selected_device_path,pipeFullPath) == 0 ) {
                            table->selected_index = table->list->size-1;
                            serial->baud_rate = *baud_rate;
                        }
                    }
                } else {
                    log_msg(LOG_ERROR, "Not implemented");
                }
            }
        } while (FindNextFileA(hFind, &findFileData));

        FindClose(hFind);
    }
#elif defined OS_POSIX
    static void device_table_fill_from_dir(object_DeviceTable * table, final char * dir, int filter_sz, char filter[][20], char * selected_device_path, int * baud_rate) {
        
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
                                char *device_path ;
                                assert(0 < strlen(dir));
                                asprintf(&device_path,"%s%s%s",dir,dir[strlen(dir)-1] == '/' ? "" : "/",namelist[namelist_n]->d_name);
                                final Device * device = device_table_add_if_not_in_by_location(table, device_path, AD_DEVICE_TYPE_SERIAL);
                                if ( device->type == AD_DEVICE_TYPE_SERIAL ) {
                                    Serial * serial = (Serial*)device;
                                    serial->detected = true;
                                }
                                if ( access(device_path,R_OK|W_OK) == 0 ) {
                                    log_msg(LOG_DEBUG, "%s: All permissions granted", device_path);
                                } else {
                                    log_msg(LOG_DEBUG, "%s: Missing permissions", device_path);
                                }
                                if ( table->selected_index == DEVICE_TABLE_NO_SELECTED ) {
                                    if ( selected_device_path != null && strcmp(selected_device_path,device_path) == 0 ) {
                                        table->selected_index = table->list->size-1;
                                        if ( device->type == AD_DEVICE_TYPE_SERIAL ) {
                                            Serial * serial = (Serial*)device;
                                            serial->baud_rate = *baud_rate;
                                        }
                                    }
                                }
                                free(device_path);
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

void device_table_set_to_undetected(object_DeviceTable * table) {
    for(int i = 0; i < table->list->size; i++) {
        Device * device = table->list->list[i];
        switch(device->type) {
            case AD_DEVICE_TYPE_SERIAL: {
                Serial * serial = (Serial*)device;
                serial->detected = false;
            } break;
            default: {
                log_msg(LOG_DEBUG, "Not implemented");
            } break;
        }        
    }
}
bool device_table_remove(object_DeviceTable * table, final Device * element) {
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
        if ( element->free == null ) {
            log_msg(LOG_WARNING, "Device not free'd since operator not set");
        } else {
            element->free(element);
        }
        return true;
    }
}
void device_table_remove_undetected(object_DeviceTable * table, bool except_network) {
    for(int i = 0; i < table->list->size; i++) {
        Device * device = table->list->list[i];
        if ( device->type == AD_DEVICE_TYPE_SERIAL ) {
            Serial * serial = (Serial*)table->list->list[i];
            if ( ! serial->detected ) {
                if ( except_network && strstr(serial->location,":") != null ) {
                    continue;
                }
                if ( i == table->selected_index ) {
                    table->selected_index = DEVICE_TABLE_NO_SELECTED;
                }
                device_table_remove(table, device);
            }
        } else {
            log_msg(LOG_ERROR, "not implemented");
        }
    }
}

void device_table_fill(object_DeviceTable * table) {
    module_debug(MODULE_SERIAL "Filling serial list (with update take care)");
    int i;
    char * selected_device_path = null;
    int baud_rate = SERIAL_DEFAULT_BAUD_RATE;
    final Device * selected_device = device_table_get_selected(table);
    if ( selected_device != null ) {
        selected_device_path = strdup(selected_device->location);
        if ( selected_device->type == AD_DEVICE_TYPE_SERIAL ) {
            Serial * serial = (Serial*)selected_device;
            baud_rate = serial->baud_rate;
        }
    }
    device_table_set_to_undetected(table);
   
    #if defined OS_WINDOWS
        device_table_fill_comports(table, selected_device_path,&baud_rate);
        device_table_fill_pipes(table, selected_device_path, &baud_rate);
    #elif defined OS_POSIX 
		char part1[][20] = {"ttys","ttyS","ttyUSB"};
	    device_table_fill_from_dir(table, "/dev/",3,part1,selected_device_path,&baud_rate);
		char part2[][20] = {""};
		device_table_fill_from_dir(table, "/dev/pts/",1,part2,selected_device_path,&baud_rate);
    #else
    #   warning Unsupported OS
    #endif

    // This may free the device after it has been openned so disabling for now
    // So function like thread_viface_cleanup_routine use the device after it has been removed/free by this list
    //device_table_remove_undetected(table, true);

    if ( selected_device_path != null ) {
        free(selected_device_path);
    }

}
