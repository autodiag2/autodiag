#ifndef __OBD_INTERFACE_H
#define __OBD_INTERFACE_H

#include "obd.h"
#include "com/serial/obd/elm327/elm327.h"

/**
 * Get current OBD protocol for which interface is currently setup
 */
OBD_PROTOCOLE_TYPE obd_interface_get_current_protocol(final OBDIFACE iface);
/**
 * Modify format of response bytes sent back by the car (if interface supiface it)
 */
bool serial_obd_interface_printing_of_spaces(final OBDIFACE iface, bool state);
/**
 * Fill interface type from information available
 * @return true if interface type has been determined else false
 */
bool obd_interface_fill_type(final OBDIFACE iface);
/**
 * Fill interface protocol
 */
bool obd_interface_fill_protocol(final OBDIFACE iface);
void obd_interface_send_wait_remote_confirmation(final OBDIFACE iface);
#endif
