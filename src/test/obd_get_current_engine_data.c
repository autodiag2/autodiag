#include "lib.h"
#include <stdio.h>
#include "com/serial/serial.h"
#include "com/serial/obd/obd.h"
#include "com/serial/obd/saej1979/saej1979.h"
#include <string.h>
#include <assert.h>

int main(int argc, char **argv) {
    OBDIFACE port = port_open(start_simulation());

//    printf("engine fuel pressure: %d kPa\n", saej1979_get_current_fuel_pressure(port));
    printf("MIL status: %s\n", saej1979_get_current_mil_status(port) ? "ON":"OFF");
    printf("Number of DTC: %d\n", saej1979_get_current_number_of_dtc(port));
    printf("Current engine type: %s\n", saej1979_get_current_engine_type_as_string(port));
    
    port_close_destruct_simulation(port);
    return 0;
}
