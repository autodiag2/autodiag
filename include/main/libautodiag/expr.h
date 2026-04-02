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
Buffer *ad_expr_reduce_invert(double target, const char *expr, int *signal_start_offset_return, char **errorReturn);
Buffer *ad_expr_reduce_invert_nearest(double target, const char *expr, double tolerance, int *signal_start_offset_return, double *actual_value_return, char **errorReturn);

#endif