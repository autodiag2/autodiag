#include "libTest.h"

bool testCarDatabaseLoad(VehicleIFace* iface) {
    final char *vehicleDirectory = installation_folder_resolve("data/vehicle/citroen/973");
    Vehicle * vehicle = db_vehicle_load_from_directory(vehicleDirectory);
    vehicle_dump(vehicle);
    assert(vehicle != null);
    assert(strcmp(vehicle->manufacturer,"Citroen") == 0);
    
    final SAEJ1979_DTC * dtc = saej1979_dtc_new();
    final DTC_DESCRIPTION * dtc_desc = dtc_description_new();
    dtc_desc->vehicle = vehicle;
    
    dtc->data[0] = 0x11 | (ISO15031_DTC_TYPE_POWERTRAIN << 6);
    dtc->data[1] = 0x52;
    dtc->data[2] = 0x00;
    
    dtc_desc_fill_from_codes_file(dtc, dtc_desc);
    testOutput("codes filled for citroen");
    dtc_description_dump(dtc_desc);
    assert(dtc_desc != null);
    return true;
}
