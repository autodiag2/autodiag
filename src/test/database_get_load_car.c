#include "database.h"
#include "ui/config.h"
#include "com/serial/obd/saej1979/saej1979.h"

int main() {
    char *carDirectory;
    asprintf(&carDirectory,"%s/data/car/citroen/973/", config_get_data_directory_safe());
    CarModel * car = car_model_load_from_directory(carDirectory);
    car_model_dump(car);
    
    final SAEJ1979_DTC * dtc = saej1979_dtc_new();
    final SAEJ1979_DTC_DESCRIPTION * dtc_desc = saej1979_dtc_description_new();
    dtc_desc->car = car;
    
    dtc->type = SAEJ1979_DTC_TYPE_POWERTRAIN;
    strcpy(dtc->number,"1152");
    saej1979_fill_dtc_from_codes_file(dtc, dtc_desc);
    printf("codes filled for citroen\n");
    saej1979_dtc_description_dump(dtc_desc);
    return 0;
}
