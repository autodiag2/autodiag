#ifndef __GLOBALS_H
#define __GLOBALS_H

#define APP_NAME "autodiag"

#define _GNU_SOURCE
#include <stdlib.h>
#include <order32.h>
#include <assert.h>
#ifdef __unix__
   #include <unistd.h>
   #ifdef _POSIX_VERSION
      #define POSIX_TERMIOS
   #endif
#endif

typedef unsigned char byte;

/**
 * Generic return values for functions.
 */
#define GENERIC_FUNCTION_ERROR -1
#define GENERIC_FUNCTION_SUCCESS 0
#define final
#define null NULL
#ifndef TRUE
 #define TRUE 1
#endif
#ifndef true
 #define true 1
#endif
#ifndef FALSE
 #define FALSE 0
#endif
#ifndef false
 #define false 0
#endif
#ifndef bool
 #define bool int
#endif
#ifndef nonnull
 #define nonnull
#endif
#ifndef nulleable
 #define nulleable
#endif

void unsupported_configuration();
void bin_dump(final char *buffer, final int size);
/**
 * Convert byte buffer to hexdump format
 */
char * buffer_to_hexdump(final char *buffer, final int size);
/**
 * reverse bits in a byte. 0x80 became 0x01
 */
unsigned char byte_reverse(unsigned char b);
/**
 * Convert a value on arbitrary given interval into a value [0;1]
 */
#define compute_normalized_of(value,min,max) ((value - min) / (max - min * 1.0))
/**
 * bitN : 0..n-1
 */
#define bitRetrieve(value,bitN) ((value & (1 << bitN)) != 0)

#endif
