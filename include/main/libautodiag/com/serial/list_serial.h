#ifndef __SERIAL_AD_LIST_H
#define __SERIAL_AD_LIST_H

#include "libautodiag/dirent.h"
#include "libautodiag/log.h"
#include "libautodiag/com/serial/serial.h"

#ifdef OS_WINDOWS
#   include "libautodiag/windows.h"
#   include <setupapi.h>
#   include <initguid.h>
#   include <devguid.h>
#   include <stdio.h>
#   include <string.h>
#   define SERIAL_AD_LIST_PIPE_PREFIX "elm327sim_"
#endif

AD_LIST_H(Serial)

OBJECT_H(SerialTable,
    list_Serial * list;
    int selected_index;
)
#define SERIAL_TABLE_NO_SELECTED -1

void serial_table_close_selected(object_SerialTable * table);
/**
 * Fill the serial list with currently detected COM ports on the system.
 */
void serial_table_fill(object_SerialTable * table);
void serial_table_free(object_SerialTable * table);
/**
 * @return NULL if not found
 */
Serial * serial_table_find_by_location(object_SerialTable * table, final char * location);
Serial * serial_table_add_if_not_in_by_location(object_SerialTable * table, char * location);

bool serial_table_remove(object_SerialTable * table, final Serial * element);
void serial_table_set_to_undetected(object_SerialTable * table);
void serial_table_remove_undetected(object_SerialTable * table, bool except_network);
/**
 * Get the selected Serial (port on which we are currently working) or NULL if no port currently selected.
 */
Serial * serial_table_get_selected(object_SerialTable * table);
void serial_table_set_selected_by_location(object_SerialTable * table, char *location);

#endif
