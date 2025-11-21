#ifndef __CUSTOM_STRING_H
#define __CUSTOM_STRING_H

#include <string.h>
#include <strings.h>
#include "libautodiag/lang/lang.h"
#include "libautodiag/lang/object.h"
#include "libautodiag/lang/list.h"
#include "compile_target.h"

#ifndef OS_POSIX
#   include <stdlib.h>
    char *strdup(const char *s);
    char *strndup(const char *s, size_t n);
#endif

bool strbeginwith(char *str, char *begin);
bool strcasebeginwith(char *str, char *begin);
typedef struct {
    char *data;
} object_string;

OBJECT_H(string)
object_string * object_string_new_from(char *data);

AD_LIST_H(object_string)

#endif
