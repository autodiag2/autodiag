#include "com/serial/obd/elm327/at/can.h"

bool serial_at_elm327_set_auto_formatting(final OBDIFACE iface, final bool state) {
    final char * command = at_command_boolean("caf",state);
    serial_send((SERIAL)iface, command);
    buffer_recycle(iface->recv_buffer);
    bool result = elm327_recv(iface) == ELM327_RESPONSE_OK;
    return result;
}
