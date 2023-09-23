#include "com/serial/at/general.h"

char * at_command(char * at_command) {
    char * res = null;
    asprintf(&res,"at%s", at_command);
    return res;
}

char * at_command_boolean(char *cmd, final bool state) {
    char *at_sps;
    asprintf(&at_sps,"%s%d", cmd, state);
    final char * command = at_command(at_sps);
    free(at_sps);
    return command;
}

int serial_at_selected_linefeeds(final bool state) {
    return serial_at_linefeeds(serial_list_get_selected(), state);
}

int serial_at_linefeeds(final nonnull SERIAL port, final bool state) {
    assert(port != null);
    final char * command = at_command_boolean("l",state);
    if ( serial_send(port, command) == SERIAL_INTERNAL_ERROR ) {
        return SERIAL_INTERNAL_ERROR;
    }
    free(command);
    final int rv = serial_recv((Serial*)port);
    if ( 0 < rv ) {
        port->linefeeds = state;
        return state;
    } else {
        return SERIAL_INTERNAL_ERROR;
    }
}

int serial_at_selected_echo(final bool state) {
    return serial_at_echo(serial_list_get_selected(), state);
}

int serial_at_echo(final nonnull SERIAL port, final bool state) {
    assert(port != null);
    final char * command = at_command_boolean("e",state);
    if ( serial_send(port, command) == SERIAL_INTERNAL_ERROR ) {
        return SERIAL_INTERNAL_ERROR;
    }
    free(command);
    if ( 0 < serial_recv((Serial*)port) ) {
        port->echo = state;
        return state;
    } else {
        return SERIAL_INTERNAL_ERROR;
    }
}

char * serial_at_print_id(final nonnull SERIAL port) {
    assert(port != null);
    final char * command = at_command("i");
    final bool result = (4 <= serial_send(port, command));
    int status;

    buffer_recycle(port->recv_buffer);
    final char * res;
    if ( 0 < serial_recv(port) ) {
        res = malloc(sizeof(char)*(port->recv_buffer->size_used+1));
        strcpy(res, port->recv_buffer->buffer);
    } else {
        res = null;
    }
    free(command);
    return res;
} 
