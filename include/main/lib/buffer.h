#ifndef __BUFFER_H
#define __BUFFER_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "log.h"
#include "bin.h"

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
void buffer_append(final Buffer * dest, final Buffer * src);
void buffer_append_bytes(final Buffer * dest, final byte *data, final int size);
void buffer_append_str(final Buffer* dest, final char *data);
bool buffer_equals(final Buffer * b1, final Buffer * b2);
/**
 * Only accept full ascii buffer, do not take in account OBD specific like printing of spaces.
 */
Buffer* ascii_n_to_bin_buffer(char * ascii, int size);
/**
 * assuming ascii is null terminated
 */
Buffer* ascii_to_bin_buffer(char * ascii);
/**
 * Get an ascii hex the given data.
 */
char* buffer_bin_to_ascii_hex(Buffer *buffer);

LIST_DEFINE_WITH_MEMBERS(BufferList,Buffer)
void BufferList_empty(BufferList* list);
void BufferList_dump(final BufferList* list);
/**
 * Obtain a new buffer with bytes initialized to random value
 */
Buffer * buffer_new_random(int sz);
/**
 * Remove the first byte of the buffer.
 */
byte buffer_extract_0(final Buffer* buffer);

char *buffer_to_string(Buffer *buffer);
Buffer * buffer_from_string(char * str);

#define BUFFER Buffer *
#define BUFFERLIST BufferList *

#endif