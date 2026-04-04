#ifndef __AD_LIB_H
#define __AD_LIB_H

// To stay compat with libautodiag-dev
#ifndef APP_NAME
#define APP_NAME "autodiag"
#endif

#include <string.h>
#define _GNU_SOURCE
#include <stdio.h>
int asprintf(char **strp, const char *fmt, ...);
#include <stdlib.h>
#include <assert.h>

#include "compile_target.h"
#include "libautodiag/lang/lang.h"
#include "libautodiag/lang/list.h"

char * ascii_escape_breaking_chars(char *str);
/**
 * For not 0 terminated strings
 */
char * ascii_escape_breaking_chars_n(char *str, size_t len);

static bool ascii_is_alphanum(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z');
}
/**
 * Generic return values for functions.
 */
#define GENERIC_FUNCTION_ERROR -1
#define GENERIC_FUNCTION_SUCCESS 0

#define GENERATE_TYPE_DUP_HEADER(type) type* type##dup(type i)
#define GENERATE_TYPE_DUP(type) GENERATE_TYPE_DUP_HEADER(type) { \
    type* v = (type*)malloc(sizeof(type)); \
    *v = i; \
    return v; \
}
GENERATE_TYPE_DUP_HEADER(int);
GENERATE_TYPE_DUP_HEADER(double);
GENERATE_TYPE_DUP_HEADER(byte);
bool * booldup(bool i);

#define AD_PTR_FREE(ptr) \
    if ( ptr != null ) { \
        free(ptr); \
        ptr = null; \
    }

#define VA_NARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
#define VA_NARGS(...) VA_NARGS_IMPL(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0)
/**
 * Avoid one declaration
 */
char * gprintf(char * format, ...);
int compat_vasprintf(char **ret, const char *fmt, va_list ap);

#include "byte.h"
#include "file.h"
#include "buffer.h"
#include "math.h"
#include "poll.h"
#include "dirent.h"
#include "thread.h"
#include "initools.h"
#include "log.h"
#include "cli_tools.h"
#include "installation.h"
#include "eventHandlerHolder.h"
#include "libautodiag/lang/all.h"
#include "libautodiag/time.h"
#include "libautodiag/handle.h"
#include "libautodiag/expr.h"

#endif
