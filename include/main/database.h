#ifndef __DATABASE_H
#define __DATABASE_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "lib/string.h"
#include "lib/lib.h"
#include "lib/log.h"
#include "lib/initools.h"

typedef struct {
    /**
     * eg BOSCH MED 17.4
     */
    char *model;
} CarECU;

typedef struct {
    /**
     * eg Citroen
     */
    char *brand;
    CarECU *ecu;
    /**
     * eg EP6DT
     */
    char *engine;
    struct {
        /**
         * Directory containing meta datas for this car
         */
        char *directory;
    } internal;
} CarModel;

#include "ui/config.h"

bool car_description_parser(char * funcData, char *key, char *value);
void car_engine_free(CarECU *engine);
CarECU* car_ecu_from_model(char *model);
CarModel* car_model_load_from_directory(char * filename);
void car_model_free(CarModel* car);
CarModel* car_model_new();
void car_model_dump(CarModel* car);

#endif
