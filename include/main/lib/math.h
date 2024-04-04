#ifndef __CUSTOM_MATH_H
#define __CUSTOM_MATH_H

#include <math.h>

#ifndef OS_WINDOWS
#   define min(a,b) (a < b ? a : b)
#   define max(a,b) (a < b ? b : a)
#endif

/**
 * Convert a value on arbitrary given interval into a value [0;1]
 */
#define compute_normalized_of(value,min,max) ((value - min) / (max - min * 1.0))

#endif
