#include "com/serial/serial_list.h"

Serial_list serial_list = { .list = null, .size = 0};
int serial_list_selected = SERIAL_LIST_NO_SELECTED;

SERIAL serial_list_get_selected() {
    if ( SERIAL_LIST_NO_SELECTED == serial_list_selected ) {
        return null;
    } else {
        assert(0 <= serial_list_selected && serial_list_selected < serial_list.size);
        return serial_list.list[serial_list_selected];
    }
}

SERIAL serial_list_add_if_not_in(SERIAL element) {
    for(int i = 0; i < serial_list.size; i++) {
        SERIAL serial = serial_list.list[i];
        if ( strcmp(serial->name,element->name) == 0 ) {
            return serial;
        }
    }
    Serial_list_append(&serial_list, element);
    return element;
}

SERIAL serial_list_add_if_not_in_by_name(char * name) {
    SERIAL serial = serial_list_find_by_name(name);
    if ( serial == null ) {
        Serial_list_append(&serial_list, serial_new());
        final SERIAL newOne = serial_list.list[serial_list.size-1];
        serial_set_name(newOne,name);
        return newOne;
    } else {
        return serial;
    }
}

LIST_APPEND_SYM(Serial_list,Serial)

void serial_list_set_selected_by_name(char *name) {
    if ( name == null ) {
        return;
    } else {
        Serial * port;
        for(int i = 0; i < serial_list.size; i++) {
            port = serial_list.list[i];
            if ( port->name != null && strcmp(port->name,name) == 0 ) {
                serial_list_selected = i;
                return;
            }
        }
    }
}

Serial * serial_list_find_by_name(final char * name) {
    if ( name == null ) {
        return null;
    } else {
        Serial * port;
        for(int i = 0; i < serial_list.size; i++) {
            port = serial_list.list[i];
            if ( port->name != null && strcmp(port->name,name) == 0 ) {
                return port;
            }
        }
        return null;
    }
}

void serial_close_selected() {
    serial_close(serial_list_get_selected());
}

void serial_list_free() {
    if ( serial_list.list != null ) {
        for(int i = 0; i < serial_list.size; i++) {
            if ( serial_list.list[i] != null ) {
                serial_free(serial_list.list[i]);
                serial_list.list[i] = null;
            }
        }
        free(serial_list.list);
        serial_list.list = null;
    }
    serial_list.size = 0;
    serial_list_selected = SERIAL_LIST_NO_SELECTED;
}
void serial_list_fill_from_dir(final char * dir, int filter_sz, char filter[][20], char * selected_serial_path, int * baud_rate) {
    #if defined OS_POSIX
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
		                        final SERIAL serial = serial_list_add_if_not_in_by_name(serial_path);
		                        serial->detected = true;
		                        if ( access(serial_path,R_OK|W_OK) == 0 ) {
		                            module_debug(MODULE_SERIAL "    All permissions granted");
		                        } else {
		                            module_debug(MODULE_SERIAL "    Missing permissions");
		                        }
		                        if ( serial_list_selected == SERIAL_LIST_NO_SELECTED ) {
		                            if ( selected_serial_path != null && strcmp(selected_serial_path,serial_path) == 0 ) {
		                                serial_list_selected = serial_list.size-1;
		                                serial->baud_rate = *baud_rate;
		                            }
		                        }
						    }
					    }
                        break;
                    }
                    default: {
                        log_msg(LOG_DEBUG, "Unknown file type=%d(%s)", namelist[namelist_n]->d_type, namelist[namelist_n]->d_name);
                    }   
                }
                free(namelist[namelist_n]);
            }
	    }
    #else
    #   warning Unsupported OS
    #endif
}
void serial_list_set_to_undetected() {
    for(int i = 0; i < serial_list.size; i++) {
        SERIAL serial = serial_list.list[i];
        serial->detected = false;
    }
}
bool serial_list_remove(final SERIAL element) {
    int index = -1;
    for(int i = 0; i < serial_list.size; i++) {
        if ( serial_list.list[i] == element ) {
            index = i;
            break;
        }
    }
    if ( index == -1 ) {
        return false;
    } else {
        if ( index < serial_list_selected ) {
            serial_list_selected --;
        }
        for ( int i = index; i < serial_list.size-1; i++) {
            serial_list.list[i] = serial_list.list[i+1];
        }
        serial_list.list = (Serial **)realloc(serial_list.list, (--serial_list.size) * sizeof(Serial*));
        serial_free(element);
        return true;
    }
}
void serial_list_remove_undetected() {
    for(int i = 0; i < serial_list.size; i++) {
        SERIAL serial = serial_list.list[i];
        if ( ! serial->detected ) {
            if ( i == serial_list_selected ) {
                serial_list_selected = SERIAL_LIST_NO_SELECTED;
            }
            serial_list_remove(serial);
        }
    }
}

void serial_list_fill() {
    module_debug(MODULE_SERIAL "Filling serial list (with update take care)");
    int i;
    char * selected_serial_path = null;
    int baud_rate = SERIAL_DEFAULT_BAUD_RATE;
    final Serial * selected_serial = serial_list_get_selected();
    if ( selected_serial != null ) {
        selected_serial_path = strdup(selected_serial->name);
        baud_rate = selected_serial->baud_rate;
    }
    serial_list_set_to_undetected();
   
    #ifdef OS_POSIX 
		char part1[][20] = {"ttys","ttyS","ttyUSB"};
	    serial_list_fill_from_dir("/dev/",3,part1,selected_serial_path,&baud_rate);
		char part2[][20] = {""};
		serial_list_fill_from_dir("/dev/pts/",1,part2,selected_serial_path,&baud_rate);
    #else
    #   warning Unsupported OS
    #endif

    serial_list_remove_undetected();
    if ( selected_serial_path != null ) {
        free(selected_serial_path);
    }

}
