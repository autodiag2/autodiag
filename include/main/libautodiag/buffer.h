#ifndef __BUFFER_H
#define __BUFFER_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "log.h"
#include "byte.h"
#include "math.h"

typedef struct {
    int size_allocated;
    int size;
    unsigned char *buffer;
} Buffer;

Buffer * buffer_new();
void buffer_free(nonnull Buffer * buffer);
Buffer * buffer_copy(nonnull Buffer* buffer);
/**
 * Ensure that there is at least size space in the buffer free space
 */
bool buffer_ensure_capacity(nonnull Buffer * buffer, int size);
/**
 * Append 0x00 to the buffer if not in
 */
void buffer_ensure_termination(nonnull Buffer * buffer);
void buffer_dump(Buffer * buffer);
/**
 * More details than buffer dump but may "conditionnal jump or move depends on uninitialized value(s)" in valgrind
 */
void buffer_debug(Buffer *buffer);
void buffer_recycle(Buffer * buffer);
/**
 * Make a left shift on the buffer (remove the first n bytes)
 */
void buffer_left_shift(final Buffer * buffer, final int shift);
/**
 * Extract part of a buffer into an other
 */
void buffer_slice(final Buffer *dest, final Buffer * src, final int index, final int size);
void buffer_append(final Buffer * dest, final Buffer * src);
void buffer_append_bytes(final Buffer * dest, final byte *data, final int size);
void buffer_append_byte(final Buffer * dest, final byte b);
void buffer_append_str(final Buffer* dest, final char *data);
void buffer_prepend_byte(final Buffer* dest, final byte b);
void buffer_prepend(final Buffer* dest, final Buffer * src);
void buffer_prepend_bytes(final Buffer* dest, final byte * data, final int size);
bool buffer_equals(final Buffer * b1, final Buffer * b2);
/**
 * Only accept full ascii buffer, do not take in account OBD specific like printing of spaces.
 */
Buffer* buffer_from_ascii_hex_n(char * ascii_hex, int size);
/**
 * assuming ascii is null terminated
 */
Buffer* buffer_from_ascii_hex(char * ascii_hex);
Buffer* buffer_from_ascii(char *ascii);
/**
 * Get an ascii hex the given data.
 * Without spaces.
 */
char* buffer_to_hex_string(Buffer *buffer);
/**
 * Convert buffer to ascii string.
 */
char * buffer_to_ascii(final Buffer *buffer);

LIST_DEFINE_WITH_MEMBERS(BufferList,Buffer)
void BufferList_empty(BufferList* list);
void BufferList_dump(final BufferList* list);
/**
 * Obtain a new buffer with bytes initialized to random value
 */
Buffer * buffer_new_random(int sz);
/**
 * Same but with a seed ensuring sequence replayability
 */
Buffer * buffer_new_random_with_seed(int sz, unsigned * seed);
Buffer * buffer_new_cycle(int sz, int percent);
/**
 * Remove the first byte of the buffer.
 */
byte buffer_extract_0(final Buffer* buffer);

/**
 * Get free space in the buffer.
 */
int buffer_get_free_space(nonnull Buffer * buffer);
/**
 * Compare two buffers
 */
bool buffer_cmp(final Buffer *buf1, final Buffer *buf2);
bool buffer_alphabet_compare(final char *ascii_hex, final char* cmp1, final char* cmp2);
void buffer_padding(final Buffer * buffer, int until, final byte pad);

#endif
