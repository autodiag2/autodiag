#ifndef __SERIAL_LIST_H
#define __SERIAL_LIST_H

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
#   define SERIAL_LIST_PIPE_PREFIX "elm327sim_"
#endif

LIST_H_STRUCT(Serial)
LIST_H_APPEND(Serial);

extern list_Serial list_serial;
extern int list_serial_selected;
#define SERIAL_LIST_NO_SELECTED -1

void serial_close_selected();
/**
 * Fill the serial list with currently detected COM ports on the system.
 */
void list_serial_fill();
void list_serial_free();
/**
 * @return NULL if not found
 */
Serial * list_serial_find_by_location(final char * location);

void serial_set_location(final Serial * port, final char *location);
bool list_serial_remove(final Serial * element);
void list_serial_set_to_undetected();
void list_serial_remove_undetected();
#endif
