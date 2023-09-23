#include "lib.h"
#include "com/serial/obd/obd.h"
#include <stdio.h>
#include "com/serial/serial.h"
#include "com/serial/obd/saej1979/saej1979.h"
#include <string.h>
#include <assert.h>

void print_id(OBDIFACE port) {
    printf("interface %s found communicating with %s protocol\n", 
        serial_at_port_interface_to_id(port->type),
        obd_get_protocol_string(port->type,port->protocol)
    );
}

void test_pid_supported(OBDIFACE port) {
    assert(saej1979_is_pid_supported(port,0));
    printf("PID full support table:\n");
    for(int i = 1; i < 0xC5; i++) {
        if ( saej1979_is_pid_supported(port,i) ) {
            printf("%02x ", i);
        }
    }
    printf("\n");
}

void get_engine_coolant_temperature(OBDIFACE port) {
    printf("engine coolant temperature: %d °C\n", saej1979_get_current_engine_coolant_temperature(port));
}

int main(int argc, char **argv) {
    log_set_level(LOG_DEBUG);
    OBDIFACE port = port_parse_open(argc,argv);
    
    print_id(port);
    test_pid_supported(port);
    get_engine_coolant_temperature(port);
    
    port_close_destruct_simulation(port);
    return 0;
}
