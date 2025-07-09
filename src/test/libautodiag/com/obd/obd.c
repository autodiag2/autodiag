#include "libTest.h"

bool testOBD(VehicleIFace* iface) {
    {
        testOutput("MIL status: %s", saej1979_data_mil_status(iface, false) ? "ON":"OFF");
        testOutput("Number of DTC: %d", saej1979_data_number_of_dtc(iface, false));
        testOutput("Current engine type: %s", saej1979_data_engine_type_as_string(iface, false));    
    }
    {
        testOutput("wup since ecu reset: %d", saej1979_data_warm_ups_since_ecu_reset(iface, false));
        testOutput("kms since ecu reset: %d km", saej1979_data_distance_since_ecu_reset(iface, false));
        testOutput("current ecu voltage: %.2fV", saej1979_data_ecu_voltage(iface, false));
        testOutput("number of dtc in ecu : %d", saej1979_data_number_of_dtc(iface, false));
    }
    return true;
}
