/**
 * KWP2000 operate at the application layer of OSI model
 * Most common configuration is to use it over CAN transport/network layer
 */
#ifndef __KWP2000_H
#define __KWP2000_H

#include "can/can.h"
#include "globals.h"

#define KWP2000_NEGATIVE_RESPONSE 0x7F
#define KWP2000_POSITIVE_RESPONSE 0x40

/**
 * Parse data from the transport layer
 */
int kwp2000_parse_data(final OBDIFACE iface);

#endif
