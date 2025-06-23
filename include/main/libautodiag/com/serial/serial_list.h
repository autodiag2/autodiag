#ifndef __SERIAL_LIST_H
#define __SERIAL_LIST_H

#include "libautodiag/dirent.h"
#include "libautodiag/log.h"
#include "libautodiag/com/serial/serial.h"

#ifdef OS_WINDOWS
#   include <windows.h>
#   include <setupapi.h>
#   include <initguid.h>
#   include <devguid.h>
#   include <stdio.h>
#   include <string.h>
#   define SERIAL_LIST_PIPE_PREFIX "elm327sim_"
#endif

LIST_H_STRUCT(Serial)
LIST_H_APPEND(Serial_list,Serial);

extern Serial_list serial_list;
extern int serial_list_selected;
#define SERIAL_LIST_NO_SELECTED -1

void serial_close_selected();
/**
 * Fill the serial list with currently detected COM ports on the system.
 */
void serial_list_fill();
void serial_list_free();
/**
 * @return NULL if not found
 */
Serial * serial_list_find_by_location(final char * location);

void serial_set_location(final Serial * port, final char *location);
bool serial_list_remove(final Serial * element);
void serial_list_set_to_undetected();
void serial_list_remove_undetected();
#endif
