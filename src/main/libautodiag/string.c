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
