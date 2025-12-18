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
char *strsep(char **stringp, const char *delim) {
    char *start = *stringp;
    char *p;

    if (start == NULL) return NULL;

    p = start;
    while (*p) {
        const char *d = delim;
        while (*d) {
            if (*p == *d) {
                *p = '\0';
                *stringp = p + 1;
                return start;
            }
            d++;
        }
        p++;
    }

    *stringp = NULL;
    return start;
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
object_string * object_string_assign(object_string * o1, object_string * o2) {
    if ( o1->data ) {
        free(o1->data);
    }
    if ( o2->data ) {
        o1->data = strdup(o2->data);
    } else {
        o1->data = null;
    }
    return o1;
}
int object_string_cmp(object_string* e1, object_string* e2) {
    return strcmp(e1->data, e2->data);
}
AD_LIST_SRC(object_string)