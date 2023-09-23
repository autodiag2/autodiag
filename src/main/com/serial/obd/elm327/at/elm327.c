#include "com/serial/obd/elm327/at/elm327.h"

bool serial_at_elm327_echo(final nonnull SERIAL port, final bool state) {
    assert(port != null);
    final char * command = at_command_boolean("e",state);
    serial_send(port, command);
    free(command);
    if ( elm327_guess_response(port->recv_buffer->buffer) == ELM327_RESPONSE_OK ) {
        port->echo = state;
    }
    log_msg("Not implemented : Should guess the interface response type", LOG_WARNING);
    return port->echo;
}

bool serial_at_elm327_set_headers(final OBDIFACE iface, final bool state) {
    final char * command = at_command_boolean("h",state);
    serial_send((SERIAL)iface, command);
    buffer_recycle(iface->recv_buffer);
    serial_recv((SERIAL)iface);
    if ( elm327_guess_response(iface->recv_buffer->buffer) == ELM327_RESPONSE_OK ) {
        iface->printing_of_headers = state;
        return true;
    } else {
        return false;
    }
}

bool serial_at_elm327_printing_of_spaces(final OBDIFACE iface, bool state) {
    final char * command = at_command_boolean("s",state);
    serial_send((SERIAL)iface, command);
    buffer_recycle(iface->recv_buffer);
    serial_recv((SERIAL)iface);
    if ( elm327_guess_response(iface->recv_buffer->buffer) == ELM327_RESPONSE_OK ) {
        iface->printing_of_spaces = state;
        return true;
    } else {
        return false;
    }
}

OBD_PROTOCOLE_TYPE serial_at_elm327_get_current_protocol(final OBDIFACE iface) {
    final char * command = at_command("dpn");
    final bool result = (3 <= serial_send((Serial*)iface, command));
    buffer_recycle(iface->recv_buffer);
    final int response = serial_recv((SERIAL)iface);
    buffer_ensure_termination(iface->recv_buffer);
    final OBD_PROTOCOLE_TYPE current_protocol;
    byte * start = iface->recv_buffer->buffer;
    byte * end = strstr(iface->recv_buffer->buffer,iface->eol);
    if ( end == null ) {
        end = iface->recv_buffer->buffer + iface->recv_buffer->size_used - 1;
    }
    *end = 0x0;
    final int chrs = strlen(start);
    final bool isAuto = (1 < chrs && *start == 'A' );
    final byte * protocolLetter;
    if ( isAuto ) {
        protocolLetter = start + 1;
    } else {
        protocolLetter = start;
    }
    current_protocol = strtol(protocolLetter,null,16);
    
    free(command);
    return current_protocol;
}

bool serial_at_elm327_calibrate_battery_voltage(final OBDIFACE iface, double voltage) {
    char *cmd;
    asprintf(&cmd,"cv%d",(int)(voltage * 100)); 
    final char * command = at_command(cmd);
    free(cmd);
    serial_send((Serial*)iface, command);
    buffer_recycle(iface->recv_buffer);
    bool result = elm327_recv(iface) == ELM327_RESPONSE_OK;
    free(command);
    return result;
}

double serial_at_elm327_get_current_battery_voltage(final OBDIFACE iface) {
    final char * command = at_command("rv");
    serial_send((Serial*)iface, command);
    buffer_recycle(iface->recv_buffer);
    final double result;
    if ( elm327_recv(iface) == ELM327_RESPONSE_DATA ) {
        buffer_ensure_termination(iface->recv_buffer);
        result = strtod(iface->recv_buffer->buffer,null);
    } else {
        result = -1;
    }
    free(command);
    return result;
}
