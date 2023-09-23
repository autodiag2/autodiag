#ifndef __SERIAL_AT_OBD_H
#define __SERIAL_AT_OBD_H

#include "com/serial/obd/obd.h"

/**
 * OBD interface id (sent back by hardware during the ati command)
 */
#define OBD_INTERFACE_ID_ELM320 "ELM320"
#define OBD_INTERFACE_ID_ELM322 "ELM322"
#define OBD_INTERFACE_ID_ELM323 "ELM323"
#define OBD_INTERFACE_ID_ELM327 "ELM327"
#define OBD_INTERFACE_ID_OBDLINK_1 "OBDLink"
#define OBD_INTERFACE_ID_OBDLINK_2 "STN1000"
#define OBD_INTERFACE_ID_OBDLINK_3 "STN11"

/**
 * Convert hardware adaptater to serial interface type.
 */
OBD_INTERFACE_TYPE serial_at_id_to_obd_interface(final char * id);
char * serial_at_port_interface_to_id(final OBD_INTERFACE_TYPE port_interface);

#endif
