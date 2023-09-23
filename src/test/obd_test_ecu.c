#include "lib.h"
#include <stdio.h>
#include "com/serial/serial.h"
#include "com/serial/obd/obd.h"
#include "com/serial/obd/saej1979/saej1979.h"
#include <string.h>
#include <assert.h>

int main(int argc, char **argv) {
    OBDIFACE port = port_open(start_simulation());

    printf("wup since ecu reset: %d\n", saej1979_get_current_warm_ups_since_ecu_reset(port));
    printf("kms since ecu reset: %d km\n", saej1979_get_current_distance_since_ecu_reset(port));
    printf("current ecu voltage: %.2fV\n", saej1979_get_current_ecu_voltage(port));
    printf("number of dtc in ecu : %d\n", saej1979_get_current_number_of_dtc(port));
    port_close_destruct_simulation(port);
    return 0;
}
