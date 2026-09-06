#ifndef __AD_CUSTOM_MATH_H
#define __AD_CUSTOM_MATH_H

#include <math.h>

#ifndef min
#   define min(a,b) (a < b ? a : b)
#endif
#ifndef max
#   define max(a,b) (a < b ? b : a)
#endif

#ifndef NAN
#   define NAN (0.0/0.0)
#endif

/**
 * Convert a value on arbitrary given interval into a value [0;1]
 */
#define compute_normalized_of(value,min,max) ((value - min) / (max - min * 1.0))

#define MATH_RAND_R_MAX 32767
static unsigned math_rand_r_seed = 0xDEADBEEF;
int math_rand_r(unsigned *seed);
double math_rand_double_1_0(unsigned * seed);
#define math_rand() math_rand_r(&math_rand_r_seed)
#define math_rand_double_1() math_rand_double_1_0(&math_rand_r_seed)

#endif
