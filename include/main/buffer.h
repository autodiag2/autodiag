#ifndef __BUFFER_H
#define __BUFFER_H

#include "globals.h"
#include "list.h"
#include "log.h"
#define _GNU_SOURCE
#include <stdio.h>

typedef struct {
    int size;
    int size_used;
    unsigned char *buffer;
} Buffer;

Buffer * buffer_new();
void buffer_free(nulleable Buffer * buffer);
/**
 * Ensure that there is at least size space in the buffer free space
 */
bool buffer_ensure_capacity(nonnull Buffer * buffer, int size);
void buffer_ensure_termination(nonnull Buffer * buffer);
void buffer_dump(Buffer * buffer);
/**
 * More details than buffer dump but may "conditionnal jump or move depends on uninitialized value(s)" in valgrind
 */
void buffer_debug(Buffer *buffer);
void buffer_recycle(Buffer * buffer);
void buffer_left_shift(final Buffer * buffer, final int shift);
void buffer_append(final Buffer * dest, final Buffer * src);
void buffer_append_str(final Buffer * dest, final byte *data, final int size);
typedef struct {
    int size;
    Buffer **list;
} BufferList;
BufferList* buffer_list_new();
void buffer_list_free(BufferList* list);
void buffer_list_append(BufferList* list, Buffer* buffer);
void buffer_list_empty(BufferList* list);
Buffer* buffer_list_remove(BufferList* list, int i);
byte buffer_extract_0(final Buffer* buffer);

#define BUFFER Buffer *
#define BUFFERLIST BufferList *

#endif
