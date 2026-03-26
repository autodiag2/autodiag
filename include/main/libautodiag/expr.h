#ifndef __AD_EXPR_H
#define __AD_EXPR_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

typedef uint8_t uint8;

double ad_expr_reduce(const uint8 *bytes, int sz, const char *expr, char **errorReturn);

#endif