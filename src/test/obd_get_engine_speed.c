#include "lib.h"
#include <stdio.h>
#include "com/serial/serial.h"
#include "com/serial/obd/obd.h"
#include "com/serial/obd/saej1979/saej1979.h"
#include <string.h>
#include <assert.h>

int main(int argc, char **argv) {
    OBDIFACE port = port_open(start_simulation());

    printf("current engine speed: %.2f rpm\n", saej1979_get_current_engine_speed(port));
    bin_dump(port->recv_buffer->buffer, port->recv_buffer->size_used);
    
    port_close_destruct_simulation(port);
    return 0;
}
