#include "libautodiag/buffer.h"
#include "libautodiag/lib.h"

void ad_buffer_fill(final Buffer * buffer, final byte value) {
    for(unsigned i = buffer->size; i < buffer->size_allocated; i++) {
        buffer->buffer[i] = value;
    }
    buffer->size = buffer->size_allocated;
}
Buffer * ad_buffer_slice(final Buffer *buffer, int from, int sz) {
    if (!buffer) return null;
    if (!buffer->buffer) return null;
    if (from < 0) return null;
    if (sz < 0) return null;
    if (buffer->size < from + sz) return null;

    Buffer *out = (Buffer*)malloc(sizeof(Buffer));
    if (!out) return null;

    out->size = sz;
    out->buffer = null;

    if (0 < sz) {
        out->buffer = (byte*)malloc((size_t)sz);
        if (!out->buffer) {
            free(out);
            return null;
        }
        memcpy(out->buffer, buffer->buffer + from, (size_t)sz);
    }

    return out;
}

bool ad_buffer_alphabet_compare(final char *ascii_hex, final char* cmp1, final char* cmp2) {
    assert(cmp1 != null);
    assert(ascii_hex != null);
    if ( strlen(ascii_hex) < strlen(cmp1)) {
        return false;
    }
    if ( cmp2 == null ) {
        return strncmp(cmp1,ascii_hex,strlen(cmp1)) == 0;
    }
    assert(strlen(cmp1) == strlen(cmp2));
    for(unsigned i = 0; i < strlen(cmp1); i++) {
        if ( cmp1[i] <= ascii_hex[i] && ascii_hex[i] <= cmp2[i] ) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}
#define AD_BUFFER_GEN_FROM_UINT(bitsize) \
    Buffer * ad_buffer_be_from_uint##bitsize(uint##bitsize##_t i) { \
        return ad_buffer_assign_uint##bitsize(ad_buffer_new(), i); \
    }
AD_BUFFER_GEN_FROM_UINT(8)
AD_BUFFER_GEN_FROM_UINT(16)
AD_BUFFER_GEN_FROM_UINT(32)
AD_BUFFER_GEN_FROM_UINT(64)
Buffer * ad_buffer_assign_uint8(Buffer * buffer, uint8_t i) {
    ad_buffer_recycle(buffer);
    ad_buffer_ensure_capacity(buffer, 1);
    buffer->buffer[0] = i & 0xFF;
    buffer->size = 1;
    return buffer;
}
Buffer * ad_buffer_assign_uint16(Buffer * buffer, uint16_t i) {
    ad_buffer_recycle(buffer);
    ad_buffer_ensure_capacity(buffer, 2);
    buffer->buffer[0] = (i >> 8) & 0xFF;
    buffer->buffer[1] = i & 0xFF;
    buffer->size = 2;
    return buffer;
}
Buffer * ad_buffer_assign_uint32(Buffer * buffer, uint32_t i) {
    ad_buffer_recycle(buffer);
    ad_buffer_ensure_capacity(buffer, 4);
    buffer->buffer[0] = (i >> 24) & 0xFF;
    buffer->buffer[1] = (i >> 16) & 0xFF;
    buffer->buffer[2] = (i >> 8) & 0xFF;
    buffer->buffer[3] = i & 0xFF;
    buffer->size = 4;
    return buffer;
}
Buffer * ad_buffer_assign_uint64(Buffer * buffer, uint64_t i) {
    ad_buffer_recycle(buffer);
    ad_buffer_ensure_capacity(buffer, 8);
    buffer->buffer[0] = (i >> 56) & 0xFF;
    buffer->buffer[1] = (i >> 48) & 0xFF;
    buffer->buffer[2] = (i >> 40) & 0xFF;
    buffer->buffer[3] = (i >> 32) & 0xFF;
    buffer->buffer[4] = (i >> 24) & 0xFF;
    buffer->buffer[5] = (i >> 16) & 0xFF;
    buffer->buffer[6] = (i >> 8) & 0xFF;
    buffer->buffer[7] = i & 0xFF;
    buffer->size = 8;
    return buffer;
}
void ad_buffer_assign(Buffer * to, Buffer * from) {
    ad_buffer_recycle(to);
    ad_buffer_memcpy(to, from->buffer, from->size);
}
int ad_buffer_cmp(final Buffer *buf1, final Buffer *buf2) {
    if ( buf1->size != buf2->size ) {
        return -1;
    }
    for(unsigned i = 0; i < buf1->size; i++) {
        if ( buf1->buffer[i] != buf2->buffer[i] ) {
            return i;
        }
    }
    return 0;
}
Buffer * ad_buffer_new_cycle(unsigned sz, int percent) {
    assert(0 <= percent && percent <= 100);
    Buffer * buffer = ad_buffer_new();
    ad_buffer_ensure_capacity(buffer, sz);
    for(unsigned i = 0; i < sz; i++) {
        buffer->buffer[i] = (byte)((percent / 100.0) * 0xFF);
    }
    buffer->size = sz;
    return buffer;
}
Buffer * ad_buffer_new_random_with_seed(unsigned sz, unsigned * seed) {
    Buffer * buffer = ad_buffer_new();
    ad_buffer_ensure_capacity(buffer, sz);
    for(unsigned i = 0; i < sz; i++) {
        buffer->buffer[i] = (byte)math_rand_r(seed);
    }
    buffer->size = sz;
    return buffer;
}
Buffer * ad_buffer_new_random(unsigned sz) {
    unsigned seed = rand();
    return ad_buffer_new_random_with_seed(sz, &seed);
}

byte ad_buffer_extract_0(final Buffer * buffer) {
    assert(0 < buffer->size);
    final byte b0 = buffer->buffer[0];
    ad_buffer_left_shift(buffer,1);
    return b0;
}

void ad_buffer_left_shift(final Buffer * buffer, final unsigned shift) {
    assert(shift <= buffer->size);
    for(int i = shift; i < buffer->size; i++) {
        buffer->buffer[i-shift] = buffer->buffer[i];
    }
    buffer->size -= shift;
}

Buffer * ad_buffer_new() {
    Buffer * buffer = (Buffer*)malloc(sizeof(Buffer));
    buffer->size_allocated = 0;
    buffer->size = 0;
    buffer->buffer = null;
    return buffer;
}
void ad_buffer_memcpy(Buffer * buffer, void * src, int sz) {
    ad_buffer_ensure_capacity(buffer, sz);
    memcpy(buffer->buffer, src, sz);
    buffer->size = sz;
}
Buffer * ad_buffer_copy(Buffer* buffer) {
    if ( buffer == null ) {
        return null;
    }
    Buffer * b = ad_buffer_new();
    ad_buffer_ensure_capacity(b,buffer->size);
    b->size = buffer->size;
    memcpy(b->buffer,buffer->buffer,b->size);
    return b;
}

void ad_buffer_free(Buffer * buffer) {
    if ( buffer != null ) {
        if ( buffer->buffer != null ) {
            free(buffer->buffer);
            buffer->buffer = null;
        }
        buffer->size_allocated = 0;
        buffer->size = 0;
        free(buffer);
    }
}

int ad_buffer_get_free_space(Buffer * buffer) {
    assert(buffer != null);
    assert(buffer->size <= buffer->size_allocated);
    return buffer->size_allocated - buffer->size;
}

bool ad_buffer_ensure_capacity(Buffer * buffer, unsigned size) {
    assert(buffer != null);
    assert(0 <= size);
    final unsigned remaining_space = ad_buffer_get_free_space(buffer);
    if ( size <= remaining_space ) {
        return false;
    } else {
        buffer->size_allocated += size - remaining_space;
        buffer->buffer = (unsigned char *)realloc(buffer->buffer,sizeof(byte)*buffer->size_allocated);
        return true;
    }
}
void ad_buffer_initialise(final Buffer * buffer, final byte value) {
    for(int i = 0; i < buffer->size; i++) {
        buffer->buffer[i] = value;
    }
}
void ad_buffer_slice_append(final Buffer *dest, final Buffer * src, final unsigned index, final unsigned size) {
    assert(size <= src->size);
    assert(dest != null);
    assert(src != null);
    assert(index+size <= src->size);
    ad_buffer_ensure_capacity(dest, size);
    memcpy(dest->buffer+dest->size, src->buffer + index, size);
    dest->size += size;
}
void ad_buffer_prepend_byte(final Buffer* dest, final byte b) {
    ad_buffer_prepend_bytes(dest, &b, 1);
}
void ad_buffer_prepend(final Buffer* dest, final Buffer * src) {
    assert(src != null);
    assert(dest != null);
    ad_buffer_prepend_bytes(dest, src->buffer, src->size);
}
void ad_buffer_prepend_bytes(final Buffer* dest, final byte * data, final unsigned size) {
    if ( size == 0 ) {
        return;
    }
    assert(dest != null);
    assert(data != null);
    ad_buffer_ensure_capacity(dest, size);
    for(unsigned i = dest->size_allocated - 1; size <= i; i--) {
        dest->buffer[i] = dest->buffer[i-size];
    }
    memcpy(dest->buffer, data, size);
    dest->size += size;
}
void ad_buffer_append_melt(final Buffer * dest, final Buffer * src) {
    ad_buffer_append(dest, src);
    ad_buffer_free(src);
}
void ad_buffer_append(final Buffer * dest, final Buffer * src) {
    assert(src != null);
    assert(dest != null);
    ad_buffer_append_bytes(dest, src->buffer, src->size);
}
void ad_buffer_append_bytes(final Buffer * dest, final byte *data, final unsigned size) {
    ad_buffer_ensure_capacity(dest, size);
    memcpy(dest->buffer+dest->size, data, size);
    dest->size += size;
}
void ad_buffer_append_byte(final Buffer * dest, final byte b) {
    ad_buffer_ensure_capacity(dest, 1);
    dest->buffer[dest->size] = b;
    dest->size ++;
}
void ad_buffer_append_str(final Buffer * dest, final char *data) {
    ad_buffer_append_bytes(dest,(byte*)data,strlen(data));
}

void ad_buffer_ensure_termination(Buffer * buffer) {
    assert(buffer != null);
    if ( 0 < buffer->size ) {
        if ( buffer->buffer[buffer->size-1] == 0 ) {
            return;
        }
    }
    ad_buffer_ensure_capacity(buffer, 1);
    buffer->buffer[buffer->size] = 0;
    buffer->size++;
}

void ad_buffer_dump(Buffer * buffer) {
    assert(buffer != null);    
    bytes_dump(buffer->buffer, buffer->size);
}

void ad_buffer_debug(Buffer *buffer) {
    assert(buffer != null);
    log_msg(LOG_DEBUG, "buffer: {size_allocated: %d, size: %d, addr: 0x%p}",buffer->size_allocated,buffer->size,buffer->buffer);
    bytes_dump(buffer->buffer, buffer->size);
}

void ad_buffer_recycle(Buffer * buffer) {
    assert(buffer != null);
    buffer->size = 0;
}

int Buffer_cmp(Buffer* e1, Buffer* e2) {
    return ad_buffer_cmp(e1, e2);
}
AD_LIST_SRC(Buffer);
AD_LIST_SRC_EMPTY(Buffer, ad_buffer_free)
bool ad_buffer_find_comparator(final Buffer *b1, final Buffer *b2) {
    return ad_buffer_cmp(b1, b2) == 0;
}
AD_LIST_SRC_FIND(Buffer, Buffer*, ad_buffer_find_comparator)

Buffer * ad_buffer_from_bytes(byte * buf, int len) {
    Buffer * b = ad_buffer_new();
    ad_buffer_memcpy(b, buf, len);
    return b;
}
Buffer * ad_buffer_from_ascii_hex_n(const char * ascii_hex, unsigned size) {
    assert(ascii_hex != null);
    Buffer * bin = ad_buffer_new();
    char *ascii_internal;
    if ( size % 2 ) {
        size ++;
        asprintf(&ascii_internal,"0%s", ascii_hex);
    } else {
        ascii_internal = strndup(ascii_hex,size);
    }
    char hex[3];
    hex[2] = 0;
    ad_buffer_ensure_capacity(bin,size/2);
    for (unsigned i = 0; i < size; i += 2) {
        memcpy(hex, ascii_internal+i, 2);
        for(unsigned j = 0; j < 2; j++) {
            if ( 'A' <= hex[j] && hex[j] <= 'F' || 
                 'a' <= hex[j] && hex[j] <= 'f' || 
                 '0' <= hex[j] && hex[j] <= '9' 
            ) {

            } else {
                ad_buffer_free(bin);
                free(ascii_internal);
                return null;
            }
        }
        bin->buffer[bin->size++] = (char)strtol(hex,null,16);
    }
    free(ascii_internal);
    return bin;
}
Buffer* ad_buffer_pad(final Buffer * buffer, final unsigned padding_until, final byte pad) {
    assert(buffer != null);
    final unsigned remaining_size_to_pad = max(padding_until - buffer->size, 0);
    ad_buffer_ensure_capacity(buffer, remaining_size_to_pad);
    for(unsigned i = buffer->size; i < padding_until; i++) {
        buffer->buffer[i] = pad;
    }
    buffer->size += remaining_size_to_pad;
    return buffer;
}
Buffer* ad_buffer_from_ascii_n(const char *ascii, int len) {
    Buffer * result = ad_buffer_new();
    ad_buffer_ensure_capacity(result, len);
    memcpy(result->buffer,ascii,len);
    result->size = len;
    return result; 
}
Buffer* ad_buffer_from_ascii(const char *ascii) {
    return ad_buffer_from_ascii_n(ascii, strlen(ascii));
}
Buffer* ad_buffer_from_ascii_hex(const char * ascii_hex) {
    return ad_buffer_from_ascii_hex_n(ascii_hex,strlen(ascii_hex));
}
Buffer* ad_buffer_from_ints_arr(const unsigned *vals, size_t n) {
    Buffer *result = ad_buffer_new();
    for (size_t i = 0; i < n; i++) {
        ad_buffer_ensure_capacity(result, result->size + 1);
        result->buffer[result->size++] = (byte)vals[i];
    }
    return result;
}
void ad_buffer_slice_non_alphanum(final Buffer *buffer) {
    int offset = 0;
    for(int i = 0; i < buffer->size; i++) {
        if ( ! ascii_is_alphanum(buffer->buffer[i]) ) {
            offset ++;
            continue;
        }
        if ( 0 < offset ) {
            buffer->buffer[i-offset] = buffer->buffer[i];
        }
    }
    buffer->size -= offset;
}
char * ad_buffer_to_ascii_espace_breaking_chars(Buffer * buffer) {
    assert(buffer != null);
    return ascii_escape_breaking_chars_n((char *)buffer->buffer, buffer->size);
}
uint8_t ad_buffer_to_be8(Buffer * buffer) {
    assert(buffer != null);
    assert(1 <= buffer->size);
    return buffer->buffer[0];
}
uint16_t ad_buffer_to_be16(Buffer * buffer) {
    assert(buffer != null);
    assert(2 <= buffer->size);
    return ((uint16_t)buffer->buffer[0] << 8) | buffer->buffer[1];
}
uint32_t ad_buffer_to_be32(Buffer * buffer) {
    assert(buffer != null);
    assert(4 <= buffer->size);
    return ((uint32_t)buffer->buffer[0] << (8 * 3)) | ((uint32_t)buffer->buffer[1] << (8 * 2)) | ((uint32_t)buffer->buffer[2] << 8) | buffer->buffer[3];
}
uint64_t ad_buffer_to_be64(Buffer * buffer) {
    assert(buffer != null);
    assert(8 <= buffer->size);
    return ((uint64_t)buffer->buffer[0] << (8 * 7)) | ((uint64_t)buffer->buffer[1] << (8 * 6)) | ((uint64_t)buffer->buffer[2] << (8 * 5)) | ((uint64_t)buffer->buffer[3] << (8 * 4)) | ((uint64_t)buffer->buffer[4] << (8 * 3)) | ((uint64_t)buffer->buffer[5] << (8 * 2)) | ((uint64_t)buffer->buffer[6] << 8) | buffer->buffer[7];
}
char * ad_buffer_to_ascii(final Buffer *buffer) {
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
char* ad_buffer_to_hex_string(Buffer *buffer) {
    if ( buffer == null || buffer->size == 0 ) {
        return strdup("");
    } else {
        return bytes_to_hex_string(buffer->buffer, buffer->size);
    }
}
char * ad_buffer_to_hexdump(final Buffer *buffer) {
    if ( buffer == null || buffer->size == 0 ) {
        return strdup("");
    } else {
        return bytes_to_hexdump(buffer->buffer, buffer->size);
    }
}
void ad_list_Buffer_dump(final ad_list_Buffer* list) {
    assert(list != null);
    AD_LIST_FOREACH(list,Buffer,buffer,
        ad_buffer_dump(buffer);
    )
}
bool ad_buffer_equals(final Buffer * b1, final Buffer * b2) {
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