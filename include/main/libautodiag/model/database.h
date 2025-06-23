#ifndef __DATABASE_H
#define __DATABASE_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "libautodiag/string.h"
#include "libautodiag/lib.h"
#include "libautodiag/log.h"
#include "libautodiag/initools.h"
#include "libautodiag/installation.h"
#include "libautodiag/model/vehicle.h"

extern list_Vehicle database;
/**
 * Load a vehicle from the filesystem to the memory and return it.
 */
Vehicle* db_vehicle_load_from_directory(char * directory);
/**
 * Load all vehicles from file system.
 */
void db_vehicle_load_in_memory();
#endif
