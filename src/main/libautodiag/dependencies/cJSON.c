#include "libautodiag/dependencies/json.h"

char * cJSON_GetStringItem(cJSON * json, char * name, char * default_value) {
    cJSON * item = cJSON_GetObjectItem(json, name);
    if ( item == null ) {
        return strdup(default_value);
    }
    char * value = cJSON_GetStringValue(item);
    return value;
}
double cJSON_GetNumberItem(cJSON * json, char * name) {
    cJSON * item = cJSON_GetObjectItem(json, name);
    if ( item == null ) {
        return NAN;
    }
    return cJSON_GetNumberValue(item);
}
bool cJSON_GetBoolItem(cJSON * json, char * name, bool default_value) {
    double value = cJSON_GetNumberItem(json, name);
    if ( value == 1.0 ) {
        return true;
    }
    if ( value == 0.0 ) {
        return false;
    }
    return default_value;
}