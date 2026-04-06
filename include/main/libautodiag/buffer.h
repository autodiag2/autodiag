#ifndef __AD_BUFFER_H
#define __AD_BUFFER_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "libautodiag/lang/list.h"
#include "log.h"
#include "byte.h"
#include "math.h"

typedef struct {
    unsigned size_allocated;
    unsigned size;
    byte *buffer;
} Buffer;

Buffer * ad_buffer_new();
void ad_buffer_free(Buffer * buffer);
/**
 * memcpy of the target into buffer (not appending copying at pointer)
 */
void ad_buffer_memcpy(Buffer * buffer, void * src, int sz);
/**
 * Produce exact copy of one buffer as a new instance.
 */
Buffer * ad_buffer_copy(Buffer* buffer);
/**
 * Produce exact copy but use an existing instance.
 */
void ad_buffer_assign(Buffer * to, Buffer * from);
Buffer* ad_buffer_assign_uint8(Buffer * buffer, uint8_t i);
/**
 * Assign a big endian 16 bits to this buffer.
 */
Buffer* ad_buffer_assign_uint16(Buffer * buffer, uint16_t i);
Buffer* ad_buffer_assign_uint32(Buffer * buffer, uint32_t i);
Buffer* ad_buffer_assign_uint64(Buffer * buffer, uint64_t i);
/**
 * Ensure that there is at least size space in the buffer free space
 */
bool ad_buffer_ensure_capacity(Buffer * buffer, unsigned size);
/**
 * Set the value for all members
 */
void ad_buffer_initialise(final Buffer * buffer, final byte value);
/**
 * For capacity allocated (free space size_allocated - size), fill with value.
 */
void ad_buffer_fill(final Buffer * buffer, final byte value);
/**
 * Append 0x00 to the buffer if not in
 */
void ad_buffer_ensure_termination(Buffer * buffer);
void ad_buffer_dump(Buffer * buffer);
/**
 * More details than buffer dump but may "conditionnal jump or move depends on uninitialized value(s)" in valgrind
 */
void ad_buffer_debug(Buffer *buffer);
void ad_buffer_recycle(Buffer * buffer);
/**
 * Make a left shift on the buffer (remove the first n bytes)
 */
void ad_buffer_left_shift(final Buffer * buffer, final unsigned shift);
/**
 * Extract part of a buffer into an other
 * [index;index+size[ if size = 0, index excluded
 * @param dest non null
 * @param src non null
 * @param index start index
 * @param size size to slice
 */
void ad_buffer_slice_append(final Buffer *dest, final Buffer * src, final unsigned index, final unsigned size);
void ad_buffer_append(final Buffer * dest, final Buffer * src);
/**
 * Same as buffer_append but destroys the src buffer.
 */
void ad_buffer_append_melt(final Buffer * dest, final Buffer * src);
void ad_buffer_append_bytes(final Buffer * dest, final byte *data, final unsigned size);
void ad_buffer_append_byte(final Buffer * dest, final byte b);
void ad_buffer_append_str(final Buffer* dest, final char *data);
void ad_buffer_append_uint8(Buffer *dest, uint8_t v);
void ad_buffer_append_uint16(Buffer *dest, uint16_t v);
void ad_buffer_append_uint32(Buffer *dest, uint32_t v);
void ad_buffer_append_uint64(Buffer *dest, uint64_t v);
void ad_buffer_prepend_byte(final Buffer* dest, final byte b);
void ad_buffer_prepend(final Buffer* dest, final Buffer * src);
void ad_buffer_prepend_bytes(final Buffer* dest, final byte * data, final unsigned size);
bool ad_buffer_equals(final Buffer * b1, final Buffer * b2);
/**
 * Only accept full ascii buffer, do not take in account OBD specific like printing of spaces.
 */
Buffer* ad_buffer_from_ascii_hex_n(const char * ascii_hex, unsigned size);
Buffer* ad_buffer_from_ascii_n(const char *ascii, int len);
/**
 * assuming ascii is null terminated
 */
Buffer* ad_buffer_from_ascii_hex(const char * ascii_hex);
Buffer* ad_buffer_from_ascii(const char *ascii);
Buffer* ad_buffer_from_ints_arr(const unsigned *vals, size_t n);
#define ad_buffer_from_ints(...) \
    ad_buffer_from_ints_arr((unsigned[]){ __VA_ARGS__ }, \
                       sizeof((unsigned[]){ __VA_ARGS__ })/sizeof(unsigned))
/**
 * Big endian
 */
Buffer * ad_buffer_be_from_uint8(uint8_t i);
/**
 * Big endian
 */
Buffer * ad_buffer_be_from_uint16(uint16_t i);
/**
 * Big endian
 */
Buffer * ad_buffer_be_from_uint32(uint32_t i);
/**
 * Big endian
 */
Buffer * ad_buffer_be_from_uint64(uint64_t i);
Buffer * ad_buffer_from_bytes(byte * buf, int len);

/**
 * Get an ascii hex the given data.
 * Without spaces.
 */
char* ad_buffer_to_hex_string(Buffer *buffer);
/**
 * Convert buffer to ascii string.
 */
char * ad_buffer_to_ascii(final Buffer *buffer);
char * ad_buffer_to_ascii_espace_breaking_chars(Buffer * buffer);
char * ad_buffer_to_hexdump(final Buffer *buffer);
uint8_t ad_buffer_to_be8(Buffer * buffer);
uint16_t ad_buffer_to_be16(Buffer * buffer);
uint32_t ad_buffer_to_be32(Buffer * buffer);
uint64_t ad_buffer_to_be64(Buffer * buffer);

AD_LIST_H(Buffer);
AD_LIST_H_FIND(Buffer, Buffer*);
AD_LIST_H_EMPTY(Buffer);
void ad_list_Buffer_dump(final ad_list_Buffer* list);
/**
 * Obtain a new buffer with bytes initialized to random value
 */
Buffer * ad_buffer_new_random(unsigned sz);
/**
 * Same but with a seed ensuring sequence replayability
 */
Buffer * ad_buffer_new_random_with_seed(unsigned sz, unsigned * seed);
Buffer * ad_buffer_new_cycle(unsigned sz, int percent);
/**
 * Remove the first byte of the buffer.
 */
byte ad_buffer_extract_0(final Buffer* buffer);
/**
 * return a copy of the buffer in input, from and to from+sz (excluded)
 */
Buffer * ad_buffer_slice(final Buffer *buffer, int from, int sz);

/**
 * Get free space in the buffer.
 */
int ad_buffer_get_free_space(Buffer * buffer);
/**
 * Compare two buffers
 */
int ad_buffer_cmp(final Buffer *buf1, final Buffer *buf2);
bool ad_buffer_alphabet_compare(final char *ascii_hex, final char* cmp1, final char* cmp2);
Buffer * ad_buffer_pad(final Buffer * buffer, unsigned until, final byte pad);
void ad_buffer_slice_non_alphanum(final Buffer *buffer);

#define __PPARG_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
#define __PPNARG_(...) __PPARG_N(__VA_ARGS__)
#define __PPRSEQ_N() 10,9,8,7,6,5,4,3,2,1,0
#define __PPNARG(...) __PPNARG_(__VA_ARGS__, __PPRSEQ_N())

#define __PPCAT(a,b) __PPCAT_I(a,b)
#define __PPCAT_I(a,b) a##b

#define __BUFFER_APPEND_1(dest,a1) ad_buffer_append_byte(dest,a1)
#define __BUFFER_APPEND_2(dest,a1,a2) __BUFFER_APPEND_1(dest,a1); ad_buffer_append_byte(dest,a2)
#define __BUFFER_APPEND_3(dest,a1,a2,a3) __BUFFER_APPEND_2(dest,a1,a2); ad_buffer_append_byte(dest,a3)
#define __BUFFER_APPEND_4(dest,a1,a2,a3,a4) __BUFFER_APPEND_3(dest,a1,a2,a3); ad_buffer_append_byte(dest,a4)
#define __BUFFER_APPEND_5(dest,a1,a2,a3,a4,a5) __BUFFER_APPEND_4(dest,a1,a2,a3,a4); ad_buffer_append_byte(dest,a5)
#define __BUFFER_APPEND_6(dest,a1,a2,a3,a4,a5,a6) __BUFFER_APPEND_5(dest,a1,a2,a3,a4,a5); ad_buffer_append_byte(dest,a6)
#define __BUFFER_APPEND_7(dest,a1,a2,a3,a4,a5,a6,a7) __BUFFER_APPEND_6(dest,a1,a2,a3,a4,a5,a6); ad_buffer_append_byte(dest,a7)
#define __BUFFER_APPEND_8(dest,a1,a2,a3,a4,a5,a6,a7,a8) __BUFFER_APPEND_7(dest,a1,a2,a3,a4,a5,a6,a7); ad_buffer_append_byte(dest,a8)
#define __BUFFER_APPEND_9(dest,a1,a2,a3,a4,a5,a6,a7,a8,a9) __BUFFER_APPEND_8(dest,a1,a2,a3,a4,a5,a6,a7,a8); ad_buffer_append_byte(dest,a9)
#define __BUFFER_APPEND_10(dest,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) __BUFFER_APPEND_9(dest,a1,a2,a3,a4,a5,a6,a7,a8,a9); max_recursion_level_reached

#define AD_BUFFER_APPEND_BYTES(dest, ...) __PPCAT(__BUFFER_APPEND_, __PPNARG(__VA_ARGS__))(dest, __VA_ARGS__)

Buffer * ad_buffer_or(Buffer * b1, Buffer * b2);
Buffer * ad_buffer_xor(Buffer * b1, Buffer * b2);

#endif
