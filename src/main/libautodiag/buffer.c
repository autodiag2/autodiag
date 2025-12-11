#include "libautodiag/buffer.h"
#include "libautodiag/lib.h"

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
    for(unsigned int i = 0; i < strlen(cmp1); i++) {
        if ( cmp1[i] <= ascii_hex[i] && ascii_hex[i] <= cmp2[i] ) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}
int buffer_cmp(final Buffer *buf1, final Buffer *buf2) {
    if ( buf1->size != buf2->size ) {
        return -1;
    }
    for(unsigned int i = 0; i < buf1->size; i++) {
        if ( buf1->buffer[i] != buf2->buffer[i] ) {
            return i;
        }
    }
    return 0;
}
Buffer * buffer_new_cycle(unsigned int sz, int percent) {
    assert(0 <= percent && percent <= 100);
    Buffer * buffer = buffer_new();
    buffer_ensure_capacity(buffer, sz);
    for(unsigned int i = 0; i < sz; i++) {
        buffer->buffer[i] = (byte)((percent / 100.0) * 0xFF);
    }
    buffer->size = sz;
    return buffer;
}
Buffer * buffer_new_random_with_seed(unsigned int sz, unsigned int * seed) {
    Buffer * buffer = buffer_new();
    buffer_ensure_capacity(buffer, sz);
    for(unsigned int i = 0; i < sz; i++) {
        buffer->buffer[i] = (byte)math_rand_r(seed);
    }
    buffer->size = sz;
    return buffer;
}
Buffer * buffer_new_random(unsigned int sz) {
    unsigned int seed = rand();
    return buffer_new_random_with_seed(sz, &seed);
}

byte buffer_extract_0(final Buffer * buffer) {
    assert(0 < buffer->size);
    final byte b0 = buffer->buffer[0];
    buffer_left_shift(buffer,1);
    return b0;
}

void buffer_left_shift(final Buffer * buffer, final unsigned int shift) {
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

Buffer * buffer_copy(Buffer* buffer) {
    assert(buffer != null);
    Buffer * b = buffer_new();
    buffer_ensure_capacity(b,buffer->size);
    b->size = buffer->size;
    memcpy(b->buffer,buffer->buffer,b->size);
    return b;
}

void buffer_free(Buffer * buffer) {
    assert(buffer != null);
    if ( buffer->buffer != null ) {
        free(buffer->buffer);
        buffer->buffer = null;
    }
    buffer->size_allocated = 0;
    buffer->size = 0;
    free(buffer);
}

int buffer_get_free_space(Buffer * buffer) {
    assert(buffer != null);
    assert(buffer->size <= buffer->size_allocated);
    return buffer->size_allocated - buffer->size;
}

bool buffer_ensure_capacity(Buffer * buffer, unsigned int size) {
    assert(buffer != null);
    assert(0 <= size);
    final unsigned int remaining_space = buffer_get_free_space(buffer);
    if ( size <= remaining_space ) {
        return false;
    } else {
        buffer->size_allocated += size - remaining_space;
        buffer->buffer = (unsigned char *)realloc(buffer->buffer,sizeof(byte)*buffer->size_allocated);
        return true;
    }
}
void buffer_initialise(final Buffer * buffer, final byte value) {
    for(int i = 0; i < buffer->size; i++) {
        buffer->buffer[i] = value;
    }
}
void buffer_slice_append(final Buffer *dest, final Buffer * src, final unsigned int index, final unsigned int size) {
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
void buffer_prepend_bytes(final Buffer* dest, final byte * data, final unsigned int size) {
    if ( size == 0 ) {
        return;
    }
    assert(dest != null);
    assert(data != null);
    buffer_ensure_capacity(dest, size);
    for(unsigned int i = dest->size_allocated - 1; size <= i; i--) {
        dest->buffer[i] = dest->buffer[i-size];
    }
    memcpy(dest->buffer, data, size);
    dest->size += size;
}
void buffer_append(final Buffer * dest, final Buffer * src) {
    assert(src != null);
    assert(dest != null);
    buffer_append_bytes(dest, src->buffer, src->size);
}
void buffer_append_bytes(final Buffer * dest, final byte *data, final unsigned int size) {
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
    buffer_append_bytes(dest,(byte*)data,strlen(data));
}

void buffer_ensure_termination(Buffer * buffer) {
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

int Buffer_cmp(Buffer* e1, Buffer* e2) {
    return buffer_cmp(e1, e2);
}
AD_LIST_SRC(Buffer)
AD_LIST_SRC_EMPTY(Buffer, buffer_free)
bool buffer_find_comparator(final Buffer *b1, final Buffer *b2) {
    return buffer_cmp(b1, b2) == 0;
}
AD_LIST_SRC_FIND(Buffer, Buffer*, buffer_find_comparator)

Buffer * buffer_from_ascii_hex_n(char * ascii_hex, unsigned int size) {
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
    for (unsigned int i = 0; i < size; i += 2) {
        memcpy(hex, ascii_internal+i, 2);
        for(unsigned int j = 0; j < 2; j++) {
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
void buffer_padding(final Buffer * buffer, final unsigned int padding_until, final byte pad) {
    assert(buffer != null);
    final unsigned int remaining_size_to_pad = max(padding_until - buffer->size, 0);
    buffer_ensure_capacity(buffer, remaining_size_to_pad);
    for(unsigned int i = buffer->size; i < padding_until; i++) {
        buffer->buffer[i] = pad;
    }
    buffer->size += remaining_size_to_pad;
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
Buffer* buffer_from_ints_arr(const unsigned int *vals, size_t n) {
    Buffer *result = buffer_new();
    for (size_t i = 0; i < n; i++) {
        buffer_ensure_capacity(result, result->size + 1);
        result->buffer[result->size++] = (byte)vals[i];
    }
    return result;
}
void buffer_slice_non_alphanum(final Buffer *buffer) {
    int offset = 0;
    for(int i = 0; i < buffer->size; i++) {
        if ( ! ascii_is_alphanum(buffer->buffer[i]) ) {
            offset ++;
            continue;
        }
        buffer->buffer[i-offset] = buffer->buffer[i];
    }
    buffer->size -= 1;
}
char * buffer_to_ascii(final Buffer *buffer) {
    if ( buffer == null ) {
        return strdup("");
    } else {
        if ( buffer->size == 0 ) {
            return strdup("");
        } else {
            assert(buffer->buffer != null);
            return bytes_to_ascii(buffer->buffer, buffer->size);
        }
    }
}
char* buffer_to_hex_string(Buffer *buffer) {
    if ( buffer == null ) {
        return strdup("");
    } else {
        return bytes_to_hex_string(buffer->buffer, buffer->size);
    }
}
void list_Buffer_dump(final list_Buffer* list) {
    assert(list != null);
    AD_LIST_FOREACH(list,Buffer,buffer,
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