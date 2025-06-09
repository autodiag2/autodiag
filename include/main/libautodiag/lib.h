#ifndef __LIB_H
#define __LIB_H

#define _GNU_SOURCE
#include <stdio.h>
int asprintf(char **strp, const char *fmt, ...);
#include <stdlib.h>
#include <assert.h>

#include "compile_target.h"
#include "lang.h"
#include "list.h"

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

#include "byte.h"
#include "file.h"
#include "buffer.h"
#include "math.h"
#include "poll.h"
#include "dirent.h"
#include "thread.h"
#include "stringlist.h"
#include "initools.h"
#include "log.h"
#include "cli_tools.h"
#include "installation.h"

#endif
