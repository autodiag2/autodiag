#ifndef __DATABASE_H
#define __DATABASE_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "log.h"
#include "initools.h"

typedef struct {
    /**
     * eg BOSCH MED 17.4
     */
    char *model;
} CarEngine;

typedef struct {
    /**
     * eg Citroen
     */
    char *brand;
    CarEngine *engine;
    /**
     * eg EP6DT
     */
    char *model;
    struct {
        /**
         * Directory containing meta datas for this car
         */
        char *directory;
    } internal;
} CarModel;

#include "ui/config.h"

bool car_description_parser(char * funcData, char *key, char *value);
void car_engine_free(CarEngine *engine);
CarEngine* car_engine_from_model(char *model);
CarModel* car_model_load_from_directory(char * filename);
void car_model_free(CarModel* car);
CarModel* car_model_new();
void car_model_dump(CarModel* car);

#endif
