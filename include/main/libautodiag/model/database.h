#ifndef __AD_DATABASE_H
#define __AD_DATABASE_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "libautodiag/string.h"
#include "libautodiag/lib.h"
#include "libautodiag/log.h"
#include "libautodiag/initools.h"
#include "libautodiag/installation.h"
#include "libautodiag/model/vehicle.h"

extern ad_list_Vehicle database;
/**
 * Load all vehicles from file system.
 */
void db_vehicle_load_in_memory();
/**
 * @param -1 to do not filter by year
 */
Vehicle *ad_db_vehicle_find_loaded(const char *manufacturer, const char *model, int year);
#endif
