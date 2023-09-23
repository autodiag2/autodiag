#ifndef __SERIAL_LIST_H
#define __SERIAL_LIST_H

#include "log.h"
#include "com/serial/serial.h"

extern Serial ** serial_list;
extern int serial_list_size;
extern int serial_list_selected;
#define SERIAL_LIST_NO_SELECTED -1

void serial_close_selected();
/**
 * Pass null as element to get a new element allocated on the heap.
 * @return element added to the list
 */
Serial * serial_list_add(Serial * element);
/**
 * Fill the serial list with currently detected COM ports on the system.
 */
void serial_list_fill();
void serial_list_free();
/**
 * @return NULL if not found
 */
Serial * serial_list_find_by_name(final char * name);
int serial_selected_send(const char *command);

void serial_set_name(final Serial * port, final char *name);
#endif
