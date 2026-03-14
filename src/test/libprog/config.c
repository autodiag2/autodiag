#include "libTest.h"

bool testCarDatabaseLoad(VehicleIFace* iface) {
    db_vehicle_load_in_memory();
    Vehicle * vehicle = ad_db_vehicle_find_loaded("Citroen", null, -1);
    vehicle_dump(vehicle);
    assert(vehicle != null);
    assert(strstr(vehicle->manufacturer,"Citroen") != null);
    
    final SAEJ1979_DTC * dtc = saej1979_dtc_new();
    final DTC_DESCRIPTION * dtc_desc = dtc_description_new();
    dtc_desc->vehicle = vehicle;
    
    dtc->data[0] = 0x11 | (ISO15031_DTC_TYPE_POWERTRAIN << 6);
    dtc->data[1] = 0x52;
    dtc->data[2] = 0x00;
    
    dtc_desc_fill_from_codes_file(dtc, dtc_desc);
    tf_test_output("codes filled for citroen");
    dtc_description_dump(dtc_desc);
    assert(dtc_desc != null);
    return true;
}
