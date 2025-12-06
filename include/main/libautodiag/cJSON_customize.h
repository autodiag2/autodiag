#ifndef __CJSON_CUSTOMIZE_H
#define __CJSON_CUSTOMIZE_H

#include "cJSON.h"
#include "libautodiag/lang/all.h"

cJSON * cJSON_GetArrayItemByStringField(cJSON * arr, char * key, char * value);

#endif