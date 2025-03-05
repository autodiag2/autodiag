#include "lib/buffer.h"

bool buffer_cmp(final Buffer *buf1, final Buffer *buf2) {
    if ( buf1->size != buf2->size ) {
        return false;
    }
    for(int i = 0; i < buf1->size; i++) {
        if ( buf1->buffer[i] != buf2->buffer[i] ) {
            return false;
        }
    }
    return true;
}

Buffer * buffer_new_random(int sz) {
    Buffer * buffer = buffer_new();
    buffer_ensure_capacity(buffer, sz);
    for(int i = 0; i < sz; i++) {
        buffer->buffer[i] = (char)rand();
    }
    buffer->size = sz;
    return buffer;
}

byte buffer_extract_0(final BUFFER buffer) {
    assert(0 < buffer->size);
    final byte b0 = buffer->buffer[0];
    buffer_left_shift(buffer,1);
    return b0;
}

void buffer_left_shift(final Buffer * buffer, final int shift) {
    assert(shift <= buffer->size);
    for(int i = shift; i < buffer->size; i++) {
        buffer->buffer[i-shift] = buffer->buffer[i];
    }
    buffer->size -= shift;
}

Buffer * buffer_new() {
    Buffer * buffer = (Buffer*)malloc(sizeof(Buffer));
    buffer->size_allocated = 0;
    buffer->size = 0;
    buffer->buffer = null;
    return buffer;
}

Buffer * buffer_copy(nonnull Buffer* buffer) {
    assert(buffer != null);
    Buffer * b = buffer_new();
    buffer_ensure_capacity(b,buffer->size);
    b->size = buffer->size;
    memcpy(b->buffer,buffer->buffer,b->size);
    return b;
}

void buffer_free(nonnull Buffer * buffer) {
    assert(buffer != null);
    if ( buffer->buffer != null ) {
        free(buffer->buffer);
        buffer->buffer = null;
    }
    buffer->size_allocated = 0;
    buffer->size = 0;
    free(buffer);
}

int buffer_get_free_space(nonnull Buffer * buffer) {
    assert(buffer != null);
    return buffer->size_allocated - buffer->size;
}

bool buffer_ensure_capacity(nonnull Buffer * buffer, int size) {
    assert(buffer != null);
    final int remaining_space = buffer_get_free_space(buffer);
    if ( size <= remaining_space ) {
        return false;
    } else {
        buffer->size_allocated += size - remaining_space;
        buffer->buffer = (unsigned char *)realloc(buffer->buffer,sizeof(byte)*buffer->size_allocated);
        return true;
    }
}
void buffer_slice(final Buffer *dest, final Buffer * src, final int index, final int size) {
    assert(size <= src->size);
    assert(dest != null);
    assert(src != null);
    assert(index < src->size);
    buffer_ensure_capacity(dest, size);
    memcpy(dest->buffer+dest->size, src->buffer, size);
    dest->size += size;
}
void buffer_append(final BUFFER dest, final BUFFER src) {
    buffer_append_bytes(dest, src->buffer, src->size);
}

void buffer_append_bytes(final BUFFER dest, final byte *data, final int size) {
    buffer_ensure_capacity(dest, size);
    memcpy(dest->buffer+dest->size, data, size);
    dest->size += size;
}
void buffer_append_byte(final BUFFER dest, final byte b) {
    buffer_ensure_capacity(dest, 1);
    dest->buffer[dest->size] = b;
    dest->size ++;
}
void buffer_append_str(final BUFFER dest, final char *data) {
    buffer_append_bytes(dest,data,strlen(data));
}

void buffer_ensure_termination(nonnull Buffer * buffer) {
    assert(buffer != null);
    if ( 0 < buffer->size ) {
        if ( buffer->buffer[buffer->size-1] == 0 ) {
            return;
        }
    }
    buffer_ensure_capacity(buffer, 1);
    buffer->buffer[buffer->size] = 0;
    buffer->size++;
}

void buffer_dump(Buffer * buffer) {
    assert(buffer != null);    
    bin_dump(buffer->buffer, buffer->size);
}

void buffer_debug(Buffer *buffer) {
    assert(buffer != null);
    log_msg(LOG_DEBUG, "buffer: {size_allocated: %d, size: %d, addr: 0x%p}",buffer->size_allocated,buffer->size,buffer->buffer);
    bin_dump(buffer->buffer, buffer->size);
}

void buffer_recycle(Buffer * buffer) {
    assert(buffer != null);
    buffer->size = 0;
}

LIST_DEFINE_MEMBERS_SYM(BufferList,Buffer)

void BufferList_empty(BUFFERLIST list) {
    assert(list != null);
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
BUFFER ascii_n_to_bin_buffer(char * ascii, int size) {
    assert(ascii != null);
    BUFFER bin = buffer_new();
    char *ascii_internal;
    if ( size % 2 ) {
        size ++;
        asprintf(&ascii_internal,"0%s", ascii);
    } else {
        ascii_internal = strndup(ascii,size);
    }
    char hex[3];
    hex[2] = 0;
    buffer_ensure_capacity(bin,size/2);
    for ( int i = 0; i < size; i += 2) {
        memcpy(hex, ascii_internal+i, 2);
        bin->buffer[bin->size++] = (char)strtol(hex,null,16);
    }
    free(ascii_internal);
    return bin;
}
Buffer* ascii_to_bin_buffer(char * ascii) {
    return ascii_n_to_bin_buffer(ascii,strlen(ascii));
}
char* buffer_bin_to_ascii_hex(Buffer *buffer) {
    assert(buffer != null);
    char *hex = (char*)malloc(sizeof(char) * (buffer->size*2 + 1));
    hex[0] = 0;
    char h[3] = {0};
    for(int i = 0; i < buffer->size; i++) {
        sprintf((char*)&h,"%02x",buffer->buffer[i]);
        strcpy(hex + i * 2, h);
    }
    return hex;
}
void BufferList_dump(final BufferList* list) {
    assert(list != null);
    LIST_FOREACH(list,Buffer,buffer,
        buffer_dump(buffer);
    )
}
bool buffer_equals(final Buffer * b1, final Buffer * b2) {
    assert(b1 != null);
    assert(b2 != null);
    if ( b1->size != b2->size ) {
        return false;
    }
    for(int i = 0; i < b1->size; i++) {
        if ( b1->buffer[i] != b2->buffer[i] ) {
            return false;
        }
    }
    return true;
}

char *buffer_to_string(Buffer *buffer) {
    int sz = buffer->size;
    int terminal_null_byte_sz = 1;
    int commas_sz = 0;
    if ( 1 < sz ) {
        commas_sz = sz - 1;
    }
    int bytes_sz = sz * 2;
    char * res = (char*)malloc((bytes_sz + commas_sz + terminal_null_byte_sz) * sizeof(char));
    *res = 0;
    for(int i = 0; i < sz; i++) {
        sprintf(res + i*3, "%02x%s", buffer->buffer[i], i + 1 < sz ? "," : "");
    }
    return res;
}

Buffer * buffer_from_string(char * str) {
    Buffer * buffer = buffer_new();
    int sz = strlen(str);
    for(int i = 0; i < sz; i += 3) {
        buffer_ensure_capacity(buffer,1);
        sscanf(str + i,"%02hhx", &buffer->buffer[buffer->size++]);
    }
    return buffer;
}
