#include "libautodiag/cJSON_customize.h"

cJSON * cJSON_GetArrayItemByStringField(cJSON * arr, char * key, char * value) {
    for(int i = 0; i < cJSON_GetArraySize(arr); i++) {
        cJSON * obj = cJSON_GetArrayItem(arr, i);
        cJSON * field = cJSON_GetObjectItem(obj, key);
        if ( field == null ) {
            return null;
        }
        if ( strcmp(field->valuestring, value) == 0 ) {
            return obj;
        }
    }
    return null;
}