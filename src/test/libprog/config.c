#include "libTest.h"

bool testCarDatabaseLoad(OBDIFace* iface) {
    final char *vehicleDirectory = installation_folder("data/car/citroen/973");
    Vehicle * vehicle = db_vehicle_load_from_directory(vehicleDirectory);
    vehicle_dump(vehicle);
    assert(vehicle != null);
    assert(strcmp(vehicle->brand,"Citroen") == 0);
    
    final SAEJ1979_DTC * dtc = saej1979_dtc_new();
    final SAEJ1979_DTC_DESCRIPTION * dtc_desc = saej1979_dtc_description_new();
    dtc_desc->vehicle = vehicle;
    
    dtc->type = ISO15031_DTC_TYPE_POWERTRAIN;
    strcpy(dtc->number,"1152");
    
    saej1979_fill_dtc_from_codes_file(dtc, dtc_desc);
    testOutput("codes filled for citroen");
    saej1979_dtc_description_dump(dtc_desc);
    assert(dtc_desc != null);
    return true;
}
