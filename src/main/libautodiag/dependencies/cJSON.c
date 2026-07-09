#include "libautodiag/dependencies/json.h"

char * cJSON_GetStringItem(cJSON * json, char * name, char * default_value) {
    cJSON * item = cJSON_GetObjectItem(json, name);
    if ( item == null ) {
        return strdup(default_value);
    }
    char * value = cJSON_GetStringValue(item);
    if ( value == null ) {
        return strdup(default_value);
    }
    return value;
}
double cJSON_GetNumberItem(cJSON * json, char * name) {
    cJSON * item = cJSON_GetObjectItem(json, name);
    if ( item == null ) {
        return NAN;
    }
    return cJSON_GetNumberValue(item);
}
bool cJSON_GetBoolItem(cJSON *json, char *name, bool default_value) {
    if (json == NULL || name == NULL) {
        return default_value;
    }

    cJSON *item = cJSON_GetObjectItemCaseSensitive(json, name);
    if (item == NULL) {
        return default_value;
    }

    if (cJSON_IsBool(item)) {
        return cJSON_IsTrue(item);
    }

    return default_value;
}