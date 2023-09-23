#include "buffer.h"

byte buffer_extract_0(final BUFFER buffer) {
    final byte b0 = buffer->buffer[0];
    buffer_left_shift(buffer,1);
    return b0;
}

void buffer_left_shift(final Buffer * buffer, final int shift) {
    for(int i = shift; i < buffer->size_used; i++) {
        buffer->buffer[i-shift] = buffer->buffer[i];
    }
    buffer->size_used -= shift;
}

Buffer * buffer_new() {
    Buffer * buffer = (Buffer*)malloc(sizeof(Buffer));
    buffer->size = 0;
    buffer->size_used = 0;
    buffer->buffer = null;
    return buffer;
}

void buffer_free(nulleable Buffer * buffer) {
    if ( buffer != null ) {
        if ( buffer->buffer != null ) {
            free(buffer->buffer);
            buffer->buffer = null;
        }
        buffer->size = 0;
        buffer->size_used = 0;
        free(buffer);
    }
}

bool buffer_ensure_capacity(nonnull Buffer * buffer, int size) {
    final int remaining_space = buffer->size - buffer->size_used;
    if ( size <= remaining_space ) {
        return false;
    } else {
        buffer->size += size - remaining_space;
        buffer->buffer = (unsigned char *)realloc(buffer->buffer,sizeof(byte)*buffer->size);
        return true;
    }
}

void buffer_append(final BUFFER dest, final BUFFER src) {
    buffer_append_str(dest, src->buffer, src->size);
}

void buffer_append_str(final BUFFER dest, final byte *data, final int size) {
    buffer_ensure_capacity(dest, size);
    memcpy(dest->buffer+dest->size_used, data, size);
    dest->size_used += size;
}

void buffer_ensure_termination(nonnull Buffer * buffer) {
    if ( 0 < buffer->size_used ) {
        if ( buffer->buffer[buffer->size_used-1] == 0 ) {
            return;
        }
    }
    buffer_ensure_capacity(buffer, 1);
    buffer->buffer[buffer->size_used] = 0;
    buffer->size_used++;
}

void buffer_dump(Buffer * buffer) {
    bin_dump(buffer->buffer, buffer->size_used);
}

void buffer_debug(Buffer *buffer) {
    char *msg;
    asprintf(&msg,"buffer: {size: %d, size_used: %d, addr: 0x%lX}",buffer->size,buffer->size_used,(unsigned long int)buffer->buffer);
    log_msg(msg, LOG_DEBUG);
    free(msg);
    bin_dump(buffer->buffer, buffer->size);
}

void buffer_recycle(Buffer * buffer) {
    buffer->size_used = 0;
}

BufferList* buffer_list_new() {
    BufferList* list = (BufferList*)malloc(sizeof(BufferList));
    LIST_NEW(list);
    return list;
}

Buffer* buffer_list_remove(BufferList* list, int i) {
    final Buffer * res = list->list[i];
    for(int j = i; j < (list->size-1); j++) {
        list->list[j] = list->list[j+1];
    }
    list->size --;
    return res;
}

void buffer_list_free(BufferList* list) {
    LIST_FREE(list);
}
void buffer_list_append(BufferList* list, Buffer* buffer) {
    list->size ++;
    list->list = (Buffer**)realloc(list->list, sizeof(Buffer*) * list->size);
    list->list[list->size-1] = buffer;
}
void buffer_list_empty(BUFFERLIST list) {
    for(int i = 0; i < list->size; i ++) {
        if ( list->list[i] != null ) {
            buffer_free(list->list[i]);
            list->list[i] = null;
        }
    }
    free(list->list);
    list->list = null;
    list->size = 0;
}
