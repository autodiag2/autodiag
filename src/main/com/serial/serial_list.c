#include "com/serial/serial_list.h"

Serial ** serial_list = NULL;
int serial_list_size = 0;
int serial_list_selected = SERIAL_LIST_NO_SELECTED;

int serial_selected_send(const char *command) {
    return serial_send(serial_list_get_selected(), command);
}

SERIAL serial_list_get_selected() {
    if ( SERIAL_LIST_NO_SELECTED == serial_list_selected ) {
        return null;
    } else {
        assert(0 <= serial_list_selected && serial_list_selected < serial_list_size);
        return serial_list[serial_list_selected];
    }
}

SERIAL serial_list_add(SERIAL element) {
    serial_list_size++;
    serial_list = (Serial **)realloc(serial_list, serial_list_size * sizeof(Serial*));
    if ( element == NULL ) {
        serial_list[serial_list_size - 1] = serial_new();
    } else {
        serial_list[serial_list_size - 1] = element;
    }
    return serial_list[serial_list_size - 1];
}

void serial_list_set_selected_by_name(char *name) {
    if ( name == null ) {
        return;
    } else {
        Serial * port;
        for(int i = 0; i < serial_list_size; i++) {
            port = serial_list[i];
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
        for(int i = 0; i < serial_list_size; i++) {
            port = serial_list[i];
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
    if ( serial_list != null ) {
        for(int i = 0; i < serial_list_size; i++) {
            if ( serial_list[i] != null ) {
                serial_free(serial_list[i]);
                serial_list[i] = null;
            }
        }
        free(serial_list);
        serial_list = null;
    }
    serial_list_size = 0;
    serial_list_selected = SERIAL_LIST_NO_SELECTED;
}

void serial_list_fill() {
    module_debug(MODULE_SERIAL "Filling serial list");
    int i;
    char * selected_serial_name = null;
    int baud_rate = BAUD_RATE_UNDEFINED;
    final Serial * selected_serial = serial_list_get_selected();
    if ( selected_serial != null ) {
        selected_serial_name = strdup(selected_serial->name);
        baud_rate = selected_serial->baud_rate;
    }
    serial_list_free();
   
    #ifdef POSIX_TERMIOS

        const int serial_number_encoding_matrix[] = {0,100,1000};
        const char locations[][20] = {"/dev/ttyS%d","/dev/ttyUSB%d","/dev/pts/%d"};
        for(int location=0;location<3;location++) {
            for(i=0;i<8;i++) {
                char *tmp;
                asprintf(&tmp,locations[location],i);
                if( access(tmp,F_OK) == 0 ) {
                    final Serial * serial = serial_list_add(NULL);
                    serial_set_name(serial, tmp);
                    serial->number = serial_number_encoding_matrix[location] + i;
                    if ( access(tmp,R_OK|W_OK) == 0 ) {
                        module_debug(MODULE_SERIAL "    All permissions granted");
                    } else {
                        module_debug(MODULE_SERIAL "    Missing permissions");
                    }
                    if ( serial_list_selected == SERIAL_LIST_NO_SELECTED ) {
                        if ( selected_serial_name != null && strcmp(selected_serial_name,serial->name) == 0 ) {
                            serial_list_selected = serial_list_size-1;
                            serial->baud_rate = baud_rate;
                        }
                    }
                }
                free(tmp);
            }
        }

    #else
        unsupported_configuration();
    #endif
    if ( selected_serial_name != null ) {
        free(selected_serial_name);
    }

}
