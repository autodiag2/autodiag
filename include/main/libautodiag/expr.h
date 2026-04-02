#ifndef __AD_EXPR_H
#define __AD_EXPR_H

#include "libautodiag/buffer.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

typedef uint8_t uint8;

/**
 * From an expression parse and compute for given input bytes.
 */
double ad_expr_reduce(const uint8 *bytes, int sz, const char *expr, char **errorReturn);
double ad_expr_reduce_buffer(Buffer * buffer, const char *expr, char **errorReturn);
#define AD_EXPR_REDUCE_INVERT_TOLERANCE_DEFAULT 5.0
/**
 * This may fail with big numbers du to the tolerance.
 */
Buffer *ad_expr_reduce_invert(double target, const char *expr, char **errorReturn);

#endif