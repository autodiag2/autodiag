#include "libautodiag/buffer.h"

bool buffer_alphabet_compare(final char *ascii_hex, final char* cmp1, final char* cmp2) {
    assert(cmp1 != null);
    assert(ascii_hex != null);
    if ( strlen(ascii_hex) < strlen(cmp1)) {
        return false;
    }
    if ( cmp2 == null ) {
        return strncmp(cmp1,ascii_hex,strlen(cmp1)) == 0;
    }
    assert(strlen(cmp1) == strlen(cmp2));
    for(int i = 0; i < strlen(cmp1); i++) {
        if ( cmp1[i] <= ascii_hex[i] && ascii_hex[i] <= cmp2[i] ) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}
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
Buffer * buffer_new_cycle(int sz, int percent) {
    Buffer * buffer = buffer_new();
    buffer_ensure_capacity(buffer, sz);
    for(int i = 0; i < sz; i++) {
        buffer->buffer[i] = (byte)((percent / 100.0) * 0xFF);
    }
    buffer->size = sz;
    return buffer;
}
Buffer * buffer_new_random_with_seed(int sz, unsigned * seed) {
    Buffer * buffer = buffer_new();
    buffer_ensure_capacity(buffer, sz);
    for(int i = 0; i < sz; i++) {
        buffer->buffer[i] = (char)rand_r(seed);
    }
    buffer->size = sz;
    return buffer;
}
Buffer * buffer_new_random(int sz) {
    unsigned seed = 1;
    return buffer_new_random_with_seed(sz, &seed);
}

byte buffer_extract_0(final Buffer * buffer) {
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
    assert(index+size <= src->size);
    buffer_ensure_capacity(dest, size);
    memcpy(dest->buffer+dest->size, src->buffer + index, size);
    dest->size += size;
}
void buffer_prepend_byte(final Buffer* dest, final byte b) {
    buffer_prepend_bytes(dest, &b, 1);
}
void buffer_prepend(final Buffer* dest, final Buffer * src) {
    assert(src != null);
    assert(dest != null);
    buffer_prepend_bytes(dest, src->buffer, src->size);
}
void buffer_prepend_bytes(final Buffer* dest, final byte * data, final int size) {
    buffer_ensure_capacity(dest, size);
    for(int i = dest->size; 0 < i; i--) {
        dest->buffer[i] = dest->buffer[i-1];
    }
    memcpy(dest->buffer, data, size);
    dest->size += size;
}
void buffer_append(final Buffer * dest, final Buffer * src) {
    assert(src != null);
    assert(dest != null);
    buffer_append_bytes(dest, src->buffer, src->size);
}

void buffer_append_bytes(final Buffer * dest, final byte *data, final int size) {
    buffer_ensure_capacity(dest, size);
    memcpy(dest->buffer+dest->size, data, size);
    dest->size += size;
}
void buffer_append_byte(final Buffer * dest, final byte b) {
    buffer_ensure_capacity(dest, 1);
    dest->buffer[dest->size] = b;
    dest->size ++;
}
void buffer_append_str(final Buffer * dest, final char *data) {
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
    bytes_dump(buffer->buffer, buffer->size);
}

void buffer_debug(Buffer *buffer) {
    assert(buffer != null);
    log_msg(LOG_DEBUG, "buffer: {size_allocated: %d, size: %d, addr: 0x%p}",buffer->size_allocated,buffer->size,buffer->buffer);
    bytes_dump(buffer->buffer, buffer->size);
}

void buffer_recycle(Buffer * buffer) {
    assert(buffer != null);
    buffer->size = 0;
}

LIST_DEFINE_MEMBERS_SYM(BufferList,Buffer)

void BufferList_empty(BufferList * list) {
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
Buffer * buffer_from_ascii_hex_n(char * ascii_hex, int size) {
    assert(ascii_hex != null);
    Buffer * bin = buffer_new();
    char *ascii_internal;
    if ( size % 2 ) {
        size ++;
        asprintf(&ascii_internal,"0%s", ascii_hex);
    } else {
        ascii_internal = strndup(ascii_hex,size);
    }
    char hex[3];
    hex[2] = 0;
    buffer_ensure_capacity(bin,size/2);
    for ( int i = 0; i < size; i += 2) {
        memcpy(hex, ascii_internal+i, 2);
        for(int j = 0; j < 2; j++) {
            if ( 'A' <= hex[j] && hex[j] <= 'F' || 
                 'a' <= hex[j] && hex[j] <= 'f' || 
                 '0' <= hex[j] && hex[j] <= '9' 
            ) {

            } else {
                buffer_free(bin);
                free(ascii_internal);
                return null;
            }
        }
        bin->buffer[bin->size++] = (char)strtol(hex,null,16);
    }
    free(ascii_internal);
    return bin;
}
void buffer_padding(final Buffer * buffer, int until, final byte pad) {
    final int toPadSz = until - buffer->size;
    buffer_ensure_capacity(buffer, toPadSz);
    for(int i = 0; i < toPadSz; i++) {
        buffer->buffer[buffer->size + i] = pad;
    }
    buffer->size += until;
}
Buffer* buffer_from_ascii(char *ascii) {
    Buffer * result = buffer_new();
    buffer_ensure_capacity(result, strlen(ascii));
    memcpy(result->buffer,ascii,strlen(ascii));
    result->size = strlen(ascii);
    return result;
}
Buffer* buffer_from_ascii_hex(char * ascii_hex) {
    return buffer_from_ascii_hex_n(ascii_hex,strlen(ascii_hex));
}
char * buffer_to_ascii(final Buffer *buffer) {
    return bytes_to_ascii(buffer->buffer, buffer->size);
}
char* buffer_to_ascii_hex(Buffer *buffer) {
    return bytes_to_ascii_hex(buffer->buffer, buffer->size);
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
