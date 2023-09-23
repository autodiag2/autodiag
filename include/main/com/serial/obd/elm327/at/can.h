#ifndef __AT_CAN_H
#define __AT_CAN_H

#include "elm327.h"

/**
 * Set can autoformatting state
 */
bool serial_at_elm327_set_auto_formatting(final OBDIFACE iface, final bool state);

#endif
