#include "com/serial/obd/interface.h"

OBD_PROTOCOLE_TYPE obd_interface_get_current_protocol(final OBDIFACE iface) {
    switch(iface->type) {
        case OBD_INTERFACE_ELM327: return serial_at_elm327_get_current_protocol(iface);
        case OBD_INTERFACE_ELM320: return OBD_PROTO_SAE_J1850_1;
        case OBD_INTERFACE_ELM322: return OBD_PROTO_SAE_J1850_2;
    }
    return OBD_PROTO_NONE;
}

bool serial_obd_interface_printing_of_spaces(final OBDIFACE iface, bool state) {
    switch(iface->type) {
        case OBD_INTERFACE_ELM327: return serial_at_elm327_printing_of_spaces(iface, state);
    }
    return false;
}

bool obd_interface_fill_type(final OBDIFACE iface) {
    module_debug(MODULE_OBD "Read id information");
    final char * response = serial_at_print_id((SERIAL)iface);
    final bool result;
    if ( response == null ) {
        iface->type = OBD_INTERFACE_UNKNOWN;
        result = false;
    } else {
        result = (iface->type = serial_at_id_to_obd_interface(response)) != OBD_INTERFACE_UNKNOWN;
    }
    free(response);
    return result;
}
bool obd_interface_fill_protocol(final OBDIFACE iface) {
    return (iface->protocol = obd_interface_get_current_protocol(iface)) != OBD_PROTO_NONE;
}

void obd_interface_send_wait_remote_confirmation(final OBDIFACE iface) {
    elm327_send_wait_remote_confirmation(iface);
}
