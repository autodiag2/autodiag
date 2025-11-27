/**
 * Extension of the base language
 */
#ifndef __LANG_H
#define __LANG_H

#define final
#define null NULL
#ifndef TRUE
 #define TRUE 1
#endif
#ifndef true
 #define true TRUE
#endif
#ifndef FALSE
 #define FALSE 0
#endif
#ifndef false
 #define false FALSE
#endif
#ifndef bool
 #define bool int
 #define bool_unset 2
#endif

typedef unsigned char byte;

/**
 * reverse bits in a byte. 0x80 became 0x01
 */
byte byte_reverse(byte b);
/**
 * bitN : 0..n-1
 */
#define bitRetrieve(value,bitN) (((value) & (1 << (bitN))) != 0)

#endif
