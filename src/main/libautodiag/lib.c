#include "libautodiag/lib.h"

GENERATE_TYPE_DUP(int)
GENERATE_TYPE_DUP(double)
GENERATE_TYPE_DUP(byte)

char * gprintf(char * format, ...) {
    char *result;
    va_list ap;
    va_start(ap, format);
    char *txt = null;
    if ( compat_vasprintf(&txt, format, ap) == -1 ) {
        log_msg(LOG_ERROR, "Fill label impossible");
        txt = null;
    }
    va_end(ap);
    return txt;
}
int compat_vasprintf(char **ret, const char *fmt, va_list ap) {
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (len < 0) return -1;
    *ret = malloc(len + 1);
    if (!*ret) return -1;
    return vsnprintf(*ret, len + 1, fmt, ap);
}

char * ascii_escape_breaking_chars(char *str) {
    size_t len = strlen(str);
    char *res = malloc(len * 4 + 1);
    char *p = res;

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];

        if (c == '\\') {
            *p++ = '\\';
            *p++ = '\\';
        } else if (c == '\r') {
            *p++ = '\\';
            *p++ = 'r';
        } else if (c == '\n') {
            *p++ = '\\';
            *p++ = 'n';
        } else if (c == '\t') {
            *p++ = '\\';
            *p++ = 't';
        } else if (c < 0x20 || c > 0x7E) {
            *p++ = '\\';
            *p++ = 'x';
            sprintf(p, "%02X", c);
            p += 2;
        } else {
            *p++ = c;
        }
    }

    *p = 0;
    return res;
}