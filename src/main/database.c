#include "database.h"

CarEngine* car_engine_new() {
    final CarEngine * ce = (CarEngine*)malloc(sizeof(CarEngine));
    ce->model = null;
    return ce;
}

bool engine_description_parser(char * funcData, char *key, char *value) {
    CarEngine* engine = (CarEngine*)funcData;
    if ( strcasecmp(key,"model") == 0 ) {
        engine->model = strdup(value);
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
    } else if ( strcasecmp(key,"model") == 0 ) {
        car->model = strdup(value);
        return true;
    } else if ( strcasecmp(key,"engine") == 0 ) {
        char * filename;
        final char * path = config_get_in_data_folder_safe("data/engine");
        asprintf(&filename, "%s/%s", path, value);
        free(path);
        CarEngine *engine = car_engine_new();
        if ( parse_ini_file(filename,engine_description_parser, engine) ) {
            car->engine = engine;
        } else {
            free(engine);
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
    printf("car: {\n");
    printf("    brand:  %s\n", car->brand);
    printf("    engine: %s\n", car->engine == null ? "null" : car->engine->model);
    printf("    model:  %s\n", car->model);
    printf("}\n");
}

CarModel* car_model_new() {
    CarModel* car = (CarModel*)malloc(sizeof(CarModel));
    car->brand = null;
    car->engine = null;
    car->model = null;
    car->internal.directory = null;
    return car;
}

void car_model_free(CarModel* car) {
    if ( car->brand != null ) {
        free(car->brand);
        car->brand = null;
    }
    if ( car->engine != null ) {
        free(car->engine);
        car->engine = null;
    }
    if ( car->model != null ) {
        free(car->model);
        car->model = null;
    }
    if ( car->internal.directory != null ) {
        free(car->internal.directory);
        car->internal.directory = null;
    }
}

CarEngine* car_engine_from_model(char *model) {
    CarEngine * ce = (CarEngine*)malloc(sizeof(CarEngine));
    ce->model = strdup(model);
    return ce;
}

void car_engine_free(CarEngine *engine) {
    if ( engine != null ) {
        free(engine);
        engine = null;
    }
}
