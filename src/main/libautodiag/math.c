#include "libautodiag/math.h"

int math_rand_r(unsigned int *seed) {
    unsigned int next = *seed;
    int result;

    next *= 1103515245;
    next += 12345;
    result = (next / 65536) % 32768;

    *seed = next;
    return result;
}