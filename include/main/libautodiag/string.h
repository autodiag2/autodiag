#ifndef __CUSTOM_STRING_H
#define __CUSTOM_STRING_H

#include <string.h>
#include <strings.h>
#include "lang.h"
#include "compile_target.h"

#ifndef OS_POSIX
#   include <stdlib.h>
    char *strdup(const char *s);
    char *strndup(const char *s, size_t n);
#endif

bool strbeginwith(char *str, char *begin);
bool strcasebeginwith(char *str, char *begin);

#endif
