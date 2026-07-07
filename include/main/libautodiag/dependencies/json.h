#ifndef __AD_DEPENDENCIES_CJSON_H
#define __AD_DEPENDENCIES_CJSON_H

#include "libautodiag/lang/all.h"
#include "libautodiag/math.h"
#include "cJSON.h"

char * cJSON_GetStringItem(cJSON * json, char * name, char * default_value);
double cJSON_GetNumberItem(cJSON * json, char * name);
bool cJSON_GetBoolItem(cJSON * json, char * name, bool default_value);

#endif