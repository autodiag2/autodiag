#ifndef __SERIAL_AT_GENERAL_H
#define __SERIAL_AT_GENERAL_H

#define _GNU_SOURCE
#include "log.h"
#include "com/serial/serial.h"

#define MODULE_SERIAL_AT "Serial AT",

char * at_command(char * at_command);
int serial_at_selected_linefeeds(final bool state);
int serial_at_linefeeds(final Serial * port, final bool state);
/**
 * Turn off echoing by the remote on the line.
 * @return the state confirmed or SERIAL_INTERNAL_ERROR
 */
int serial_at_selected_echo(final bool state);
int serial_at_echo(final Serial * port, final bool state);
char * serial_at_print_id(final Serial * port);
char * at_command_boolean(char *at_command, final bool state);

#endif
