#include "libautodiag/com/obd/database.h"

VehicleList database = { .list = null, .size = 0};

bool db_ecu_description_parser(char * funcData, char *key, char *value) {
    ECU* ecu = (ECU*)funcData;
    if ( strcasecmp(key,"model") == 0 ) {
        ecu->model = strdup(value);
        return true;
    } else if ( strcasecmp(key,"brand") == 0 ) {
        return true;
    }
    return false;
}

bool db_vehicle_description_parser(char * funcData, char *key, char *value) {
    Vehicle *vehicle = (Vehicle*)funcData;
    if ( strcasecmp(key,"brand") == 0 ) {
        vehicle->brand = strdup(value);
        return true;
    } else if ( strcasecmp(key,"engine") == 0 ) {
        vehicle->engine = strdup(value);
        return true;
    } else if ( strcasecmp(key,"ecu") == 0 ) {
        log_msg(LOG_ERROR, "Need to set the ecu model to: %s", value);
        return true;
    }
    return false;
}
Vehicle* db_vehicle_load_from_database(char * directory) {
    for(int i = 0; i < database.size; i++) {
        Vehicle * vehicle = database.list[i];
        if ( strcmp(vehicle->internal.directory, directory) == 0 ) {
            return vehicle;
        }
    }
    return null;
}
Vehicle* db_vehicle_load_from_directory(char * directory) {
    Vehicle * vehicle = db_vehicle_load_from_database(directory);
    if ( vehicle != null ) {
        return vehicle;
    }
    char *filename;
    asprintf(&filename, "%s/desc.ini", directory);
    vehicle = vehicle_new();
    if ( parse_ini_file(filename,db_vehicle_description_parser, vehicle) ) {
        vehicle->internal.directory = strdup(directory);
        VehicleList_append(&database, vehicle);
    } else {
        free(vehicle);
        vehicle = null;
    }
    free(filename);
    return vehicle;
}
