/**
 * A Controller Area Network (CAN) bus is designed to allow microcontrollers and devices to communicate.
 */
#ifndef __CAN_H
#define __CAN_H

#include "com/serial/obd/obd.h"
#include "iso15765.h"
#include "globals.h"
#include "buffer.h"

/**
 * Tell if OBD interface is can or not (this change the message format)
 */
bool obd_iface_is_can(final OBDIFACE iface);

#endif
