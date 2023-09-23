#include "com/serial/at/obd/obd.h"

OBD_INTERFACE_TYPE serial_at_id_to_obd_interface(final char * id) {
    if ( id == null )
        return OBD_INTERFACE_UNKNOWN;
    if (strncmp(id, OBD_INTERFACE_ID_ELM320, strlen(OBD_INTERFACE_ID_ELM320)) == 0)
        return OBD_INTERFACE_ELM320;
    if (strncmp(id, OBD_INTERFACE_ID_ELM322, strlen(OBD_INTERFACE_ID_ELM322)) == 0)
        return OBD_INTERFACE_ELM322;
    if (strncmp(id, OBD_INTERFACE_ID_ELM323, strlen(OBD_INTERFACE_ID_ELM323)) == 0)
        return OBD_INTERFACE_ELM323;
    if (strncmp(id, OBD_INTERFACE_ID_ELM327, strlen(OBD_INTERFACE_ID_ELM327)) == 0)
        return OBD_INTERFACE_ELM327;
    if (strncmp(id, OBD_INTERFACE_ID_OBDLINK_1, strlen(OBD_INTERFACE_ID_OBDLINK_1)) == 0 ||
       strncmp(id, OBD_INTERFACE_ID_OBDLINK_2, strlen(OBD_INTERFACE_ID_OBDLINK_2)) == 0 ||
       strncmp(id, OBD_INTERFACE_ID_OBDLINK_3, strlen(OBD_INTERFACE_ID_OBDLINK_3)) == 0)
        return OBD_INTERFACE_OBDLINK;
    if (strncmp(id, "SCANTOOL.NET", 12) == 0)
        return OBD_INTERFACE_SCANTOOL;
    if (strcmp(id, "OBDIItoRS232Interpreter") == 0)
        return OBD_INTERFACE_GENERIC;  
    return OBD_INTERFACE_UNKNOWN;
}

char * serial_at_port_interface_to_id(final OBD_INTERFACE_TYPE iface_type) {
    switch(iface_type) {
        case OBD_INTERFACE_ELM320: return OBD_INTERFACE_ID_ELM320;
        case OBD_INTERFACE_ELM322: return OBD_INTERFACE_ID_ELM322;
        case OBD_INTERFACE_ELM323: return OBD_INTERFACE_ID_ELM323;
        case OBD_INTERFACE_ELM327: return OBD_INTERFACE_ID_ELM327;
        default: return "Unknown";
    }
}
