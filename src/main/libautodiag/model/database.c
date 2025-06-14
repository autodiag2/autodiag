#include "libautodiag/model/database.h"

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
    assert(directory != null);
    for(int i = 0; i < database.size; i++) {
        Vehicle * vehicle = database.list[i];
        if ( vehicle->internal.directory != null ) {
            if ( strcmp(vehicle->internal.directory, directory) == 0 ) {
                return vehicle;
            }
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
void db_vehicle_load_in_memory_fs_recurse(final char*path) {
    #if defined OS_POSIX
        DIRENT **namelist;   
        final int namelist_n = scandir(path, &namelist,NULL,&alphasort);

        if ( namelist_n == -1 ) {
            char *path_str;
            asprintf(&path_str,"scandir error for %s\n", path);
            perror(path_str);
            free(path_str);
        } else {
            bool isPathCarDirectory = false;
            while (namelist_n--) {
                if ( ! isPathCarDirectory ) {
                    switch(namelist[namelist_n]->d_type) {
                        case DT_DIR: {
                            if ( strcmp(namelist[namelist_n]->d_name, ".") != 0 
                            && strcmp(namelist[namelist_n]->d_name, "..") != 0 ) {
                                char *nextPath;
                                asprintf(&nextPath, "%s/%s", path, namelist[namelist_n]->d_name);
                                db_vehicle_load_in_memory_fs_recurse(nextPath);
                                free(nextPath);
                            }
                            break;
                        }
                        case DT_REG:
                        case DT_BLK:
                        case DT_FIFO:
                        case DT_CHR: {
                            isPathCarDirectory = true;
                            db_vehicle_load_from_directory(path);
                            break;
                        }
                        default: {
                            log_msg(LOG_DEBUG, "Unknown file type=%d(%s)", namelist[namelist_n]->d_type, namelist[namelist_n]->d_name);
                        }   
                    }
                }
                free(namelist[namelist_n]);
            }
        }
    #else
    #   warning Unsupported OS        
    #endif
}
void db_vehicle_load_in_memory() {
    VehicleList_empty(&database);
    final char * basepath = installation_folder("data/car/");
    db_vehicle_load_in_memory_fs_recurse(basepath);
    free(basepath);
}