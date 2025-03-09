#include "database.h"

CarECU* car_ecu_new() {
    final CarECU * ce = (CarECU*)malloc(sizeof(CarECU));
    ce->model = null;
    return ce;
}

bool ecu_description_parser(char * funcData, char *key, char *value) {
    CarECU* ecu = (CarECU*)funcData;
    if ( strcasecmp(key,"model") == 0 ) {
        ecu->model = strdup(value);
        return true;
    } else if ( strcasecmp(key,"brand") == 0 ) {
        return true;
    }
    return false;
}

bool car_description_parser(char * funcData, char *key, char *value) {
    CarModel *car = (CarModel*)funcData;
    if ( strcasecmp(key,"brand") == 0 ) {
        car->brand = strdup(value);
        return true;
    } else if ( strcasecmp(key,"engine") == 0 ) {
        car->engine = strdup(value);
        return true;
    } else if ( strcasecmp(key,"ecu") == 0 ) {
        char * filename;
        final char * path = config_get_in_data_folder_safe("data/ecu");
        asprintf(&filename, "%s/%s", path, value);
        free(path);
        CarECU *ecu = car_ecu_new();
        if ( parse_ini_file(filename,ecu_description_parser, ecu) ) {
            car->ecu = ecu;
        } else {
            free(ecu);
        }
        free(filename);
        return true;
    }
    return false;
}

CarModel* car_model_load_from_directory(char * directory) {
    char *filename;
    asprintf(&filename, "%s/desc.ini", directory);
    final CarModel *car = car_model_new();
    if ( parse_ini_file(filename,car_description_parser, car) ) {
        car->internal.directory = strdup(directory);
    } else {
        free(car);
        car = null;
    }
    free(filename);
    return car;
}

void car_model_dump(CarModel* car) {
    assert(car != null);
    printf("car: {\n");
    printf("    brand:  %s\n", car->brand);
    printf("    ecu: %s\n", car->ecu == null ? "null" : car->ecu->model);
    printf("    engine:  %s\n", car->engine);
    printf("}\n");
}

CarModel* car_model_new() {
    CarModel* car = (CarModel*)malloc(sizeof(CarModel));
    car->brand = null;
    car->ecu = null;
    car->engine = null;
    car->internal.directory = null;
    return car;
}

void car_model_free(CarModel* car) {
    if ( car->brand != null ) {
        free(car->brand);
        car->brand = null;
    }
    if ( car->ecu != null ) {
        free(car->ecu);
        car->ecu = null;
    }
    if ( car->engine != null ) {
        free(car->engine);
        car->engine = null;
    }
    if ( car->internal.directory != null ) {
        free(car->internal.directory);
        car->internal.directory = null;
    }
}

CarECU* car_ecu_from_model(char *model) {
    CarECU * ce = (CarECU*)malloc(sizeof(CarECU));
    ce->model = strdup(model);
    return ce;
}

void car_ecu_free(CarECU *engine) {
    if ( engine != null ) {
        free(engine);
        engine = null;
    }
}
