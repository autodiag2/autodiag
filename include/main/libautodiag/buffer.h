#ifndef __BUFFER_H
#define __BUFFER_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "libautodiag/lang/list.h"
#include "log.h"
#include "byte.h"
#include "math.h"

typedef struct {
    unsigned int size_allocated;
    unsigned int size;
    byte *buffer;
} Buffer;

Buffer * buffer_new();
void buffer_free(Buffer * buffer);
Buffer * buffer_copy(Buffer* buffer);
/**
 * Ensure that there is at least size space in the buffer free space
 */
bool buffer_ensure_capacity(Buffer * buffer, unsigned int size);
/**
 * Set the value for all members
 */
void buffer_initialise(final Buffer * buffer, final byte value);
/**
 * Append 0x00 to the buffer if not in
 */
void buffer_ensure_termination(Buffer * buffer);
void buffer_dump(Buffer * buffer);
/**
 * More details than buffer dump but may "conditionnal jump or move depends on uninitialized value(s)" in valgrind
 */
void buffer_debug(Buffer *buffer);
void buffer_recycle(Buffer * buffer);
/**
 * Make a left shift on the buffer (remove the first n bytes)
 */
void buffer_left_shift(final Buffer * buffer, final unsigned int shift);
/**
 * Extract part of a buffer into an other
 * [index;index+size[ if size = 0, index excluded
 * @param dest non null
 * @param src non null
 * @param index start index
 * @param size size to slice
 */
void buffer_slice_append(final Buffer *dest, final Buffer * src, final unsigned int index, final unsigned int size);
void buffer_append(final Buffer * dest, final Buffer * src);
void buffer_append_bytes(final Buffer * dest, final byte *data, final unsigned int size);
void buffer_append_byte(final Buffer * dest, final byte b);
void buffer_append_str(final Buffer* dest, final char *data);
void buffer_prepend_byte(final Buffer* dest, final byte b);
void buffer_prepend(final Buffer* dest, final Buffer * src);
void buffer_prepend_bytes(final Buffer* dest, final byte * data, final unsigned int size);
bool buffer_equals(final Buffer * b1, final Buffer * b2);
/**
 * Only accept full ascii buffer, do not take in account OBD specific like printing of spaces.
 */
Buffer* buffer_from_ascii_hex_n(char * ascii_hex, unsigned int size);
/**
 * assuming ascii is null terminated
 */
Buffer* buffer_from_ascii_hex(char * ascii_hex);
Buffer* buffer_from_ascii(char *ascii);
Buffer* buffer_from_ints_arr(const unsigned int *vals, size_t n);
#define buffer_from_ints(...) \
    buffer_from_ints_arr((unsigned int[]){ __VA_ARGS__ }, \
                       sizeof((unsigned int[]){ __VA_ARGS__ })/sizeof(unsigned int))
/**
 * Get an ascii hex the given data.
 * Without spaces.
 */
char* buffer_to_hex_string(Buffer *buffer);
/**
 * Convert buffer to ascii string.
 */
char * buffer_to_ascii(final Buffer *buffer);

AD_LIST_H(Buffer);
AD_LIST_H_FIND(Buffer, Buffer*);
void list_Buffer_empty(list_Buffer* list);
void list_Buffer_dump(final list_Buffer* list);
/**
 * Obtain a new buffer with bytes initialized to random value
 */
Buffer * buffer_new_random(unsigned int sz);
/**
 * Same but with a seed ensuring sequence replayability
 */
Buffer * buffer_new_random_with_seed(unsigned int sz, unsigned int * seed);
Buffer * buffer_new_cycle(unsigned int sz, int percent);
/**
 * Remove the first byte of the buffer.
 */
byte buffer_extract_0(final Buffer* buffer);

/**
 * Get free space in the buffer.
 */
int buffer_get_free_space(Buffer * buffer);
/**
 * Compare two buffers
 */
int buffer_cmp(final Buffer *buf1, final Buffer *buf2);
bool buffer_alphabet_compare(final char *ascii_hex, final char* cmp1, final char* cmp2);
void buffer_padding(final Buffer * buffer, unsigned int until, final byte pad);

#define __PPARG_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
#define __PPNARG_(...) __PPARG_N(__VA_ARGS__)
#define __PPRSEQ_N() 10,9,8,7,6,5,4,3,2,1,0
#define __PPNARG(...) __PPNARG_(__VA_ARGS__, __PPRSEQ_N())

#define __PPCAT(a,b) __PPCAT_I(a,b)
#define __PPCAT_I(a,b) a##b

#define __BUFFER_APPEND_1(dest,a1) buffer_append_byte(dest,a1)
#define __BUFFER_APPEND_2(dest,a1,a2) __BUFFER_APPEND_1(dest,a1); buffer_append_byte(dest,a2)
#define __BUFFER_APPEND_3(dest,a1,a2,a3) __BUFFER_APPEND_2(dest,a1,a2); buffer_append_byte(dest,a3)
#define __BUFFER_APPEND_4(dest,a1,a2,a3,a4) __BUFFER_APPEND_3(dest,a1,a2,a3); buffer_append_byte(dest,a4)
#define __BUFFER_APPEND_5(dest,a1,a2,a3,a4,a5) __BUFFER_APPEND_4(dest,a1,a2,a3,a4); buffer_append_byte(dest,a5)
#define __BUFFER_APPEND_6(dest,a1,a2,a3,a4,a5,a6) __BUFFER_APPEND_5(dest,a1,a2,a3,a4,a5); buffer_append_byte(dest,a6)
#define __BUFFER_APPEND_7(dest,a1,a2,a3,a4,a5,a6,a7) __BUFFER_APPEND_6(dest,a1,a2,a3,a4,a5,a6); buffer_append_byte(dest,a7)
#define __BUFFER_APPEND_8(dest,a1,a2,a3,a4,a5,a6,a7,a8) __BUFFER_APPEND_7(dest,a1,a2,a3,a4,a5,a6,a7); buffer_append_byte(dest,a8)
#define __BUFFER_APPEND_9(dest,a1,a2,a3,a4,a5,a6,a7,a8,a9) __BUFFER_APPEND_8(dest,a1,a2,a3,a4,a5,a6,a7,a8); buffer_append_byte(dest,a9)
#define __BUFFER_APPEND_10(dest,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) __BUFFER_APPEND_9(dest,a1,a2,a3,a4,a5,a6,a7,a8,a9); max_recursion_level_reached

#define BUFFER_APPEND_BYTES(dest, ...) __PPCAT(__BUFFER_APPEND_, __PPNARG(__VA_ARGS__))(dest, __VA_ARGS__)

#endif
