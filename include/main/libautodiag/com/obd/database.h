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
#include "libautodiag/com/obd/vehicle.h"

Vehicle* db_vehicle_load_from_directory(char * directory);
extern VehicleList database;

#endif
