#ifndef __AT_ELM327_H
#define __AT_ELM327_H

#include "com/serial/obd/obd.h"

OBD_PROTOCOLE_TYPE serial_at_elm327_get_current_protocol(final OBDIFACE iface);
/**
 * @return true on confirmation from the remote, false on missing confirmation
 */
bool serial_at_elm327_printing_of_spaces(final OBDIFACE iface, bool state);
/**
 * @return current voltage level or -1 on error
 */
double serial_at_elm327_get_current_battery_voltage(final OBDIFACE iface);
/**
 * Calibrate current battery voltage
 */
bool serial_at_elm327_calibrate_battery_voltage(final OBDIFACE iface, double voltage);
/**
 * Set printing of headers in responses
 */
bool serial_at_elm327_set_headers(final OBDIFACE iface, final bool state);

#include "can.h"

#endif
