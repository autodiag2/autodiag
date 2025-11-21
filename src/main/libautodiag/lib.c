#include "libautodiag/lib.h"

GENERATE_TYPE_DUP(int)
GENERATE_TYPE_DUP(double)
GENERATE_TYPE_DUP(byte)

char * ascii_escape_breaking_chars(char *str) {
    size_t len = strlen(str);
    char *res = malloc(len * 2 + 1);
    char *p = res;

    for (size_t i = 0; i < len; i++) {
        if (str[i] == '\r') {
            *p++ = '\\';
            *p++ = 'r';
        } else {
            *p++ = str[i];
        }
    }

    *p = 0;
    return res;
}