/**
 * This file detect configuration of the system this is a compilation tool only
 */
#ifndef __OS_H
#define __OS_H

/**
 * Define OS_* compilation variables
 */
#if defined __unix__ || defined __APPLE__
#   define OS_UNIX
#   include <unistd.h>
#   if defined _POSIX_VERSION || defined _POSIX_C_SOURCE || defined _POSIX_SOURCE
#       define OS_POSIX
#   endif
#elif defined _WIN32 || defined WIN32
#   define OS_WINDOWS
#else
#   error "Target OS not supported"
#endif

#include <limits.h>
#include <stdint.h>

#if CHAR_BIT != 8
#error "unsupported char size"
#endif

enum
{
    O32_LITTLE_ENDIAN = 0x03020100ul,
    O32_BIG_ENDIAN = 0x00010203ul,
    O32_PDP_ENDIAN = 0x01000302el,
    O32_HONEYWELL_ENDIAN = 0x02030001ul
};

static const union { unsigned char bytes[4]; uint32_t value; } o32_host_order = { { 0, 1, 2, 3 } };

#define O32_HOST_ORDER (o32_host_order.value)

#endif
