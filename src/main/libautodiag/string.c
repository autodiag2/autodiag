#include "libautodiag/string.h"

#ifndef OS_POSIX

char *strdup(const char *s) {
    return strndup(s, strlen(s));
}

char *strndup(const char *s, size_t n) {
    char * res = (char*)malloc(sizeof(n+1) * sizeof(char));
    if ( res == null ) {
        return null;
    } else {
        memcpy(res,s,n);
        res[n] = 0;
        return res;
    }
}

#endif

bool strbeginwith(char *str, char *begin) {
    return strncmp(str,begin,strlen(begin)) == 0;
}

bool strcasebeginwith(char *str, char *begin) {
    return strncasecmp(str,begin,strlen(begin)) == 0;
}

object_string * object_string_new() {
    object_string * object = (object_string *) malloc(sizeof(object_string));
    object->data = null;
    return object;
}
void object_string_free(object_string * object) {
    if ( object->data != null ) {
        free(object->data);
        object->data = null;
    }
    free(object);
}
object_string * object_string_new_from(char *data) {
    object_string * s = object_string_new();
    s->data = strdup(data);
    return s;
}
LIST_SRC(object_string)