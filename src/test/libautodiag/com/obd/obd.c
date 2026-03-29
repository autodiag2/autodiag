#include "libTest.h"

bool testOBD(VehicleIFace* iface) {
    {
        tf_test_output("MIL status: %s", saej1979_data_mil_status(iface, AD_SAEJ1979_DATA_FRAME_LIVE) ? "ON":"OFF");
        tf_test_output("Number of DTC: %d", saej1979_data_number_of_dtc(iface, AD_SAEJ1979_DATA_FRAME_LIVE));
        tf_test_output("Current engine type: %s", saej1979_data_engine_type_as_string(iface, AD_SAEJ1979_DATA_FRAME_LIVE));    
    }
    {
        tf_test_output("wup since ecu reset: %d", saej1979_data_warm_ups_since_ecu_reset(iface, AD_SAEJ1979_DATA_FRAME_LIVE));
        tf_test_output("kms since ecu reset: %d km", saej1979_data_distance_since_ecu_reset(iface, AD_SAEJ1979_DATA_FRAME_LIVE));
        tf_test_output("current ecu voltage: %.2fV", saej1979_data_ecu_voltage(iface, AD_SAEJ1979_DATA_FRAME_LIVE));
        tf_test_output("number of dtc in ecu : %d", saej1979_data_number_of_dtc(iface, AD_SAEJ1979_DATA_FRAME_LIVE));
    }
    return true;
}
