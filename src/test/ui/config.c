#include "libTest.h"

bool testCarDatabaseLoad(OBDIFace* iface) {
    char *carDirectory;
    asprintf(&carDirectory,"%s/data/car/citroen/973/", config_get_data_directory_safe());
    CarModel * car = car_model_load_from_directory(carDirectory);
    car_model_dump(car);
    assert(car != null);
    assert(strcmp(car->brand,"Citroen") == 0);
    
    final SAEJ1979_DTC * dtc = saej1979_dtc_new();
    final SAEJ1979_DTC_DESCRIPTION * dtc_desc = saej1979_dtc_description_new();
    dtc_desc->car = car;
    
    dtc->type = ISO15031_DTC_TYPE_POWERTRAIN;
    strcpy(dtc->number,"1152");
    
    saej1979_fill_dtc_from_codes_file(dtc, dtc_desc);
    testOutput("codes filled for citroen");
    saej1979_dtc_description_dump(dtc_desc);
    assert(dtc_desc != null);
    return true;
}
