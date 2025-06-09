#include "libautodiag/byte.h"

char * bytes_to_ascii(final byte *buffer, final int size) {
    assert(buffer != null);
    char * res = (char*) malloc(sizeof(char) * (size+1));
    memcpy(res, buffer, size);
    res[size] = 0;
    return res;
}
char * bytes_to_ascii_hex(final byte *buffer, final int size) {
    assert(buffer != null);
    char *hex = (char*)malloc(sizeof(char) * (size*2 + 1));
    hex[0] = 0;
    char h[3] = {0};
    for(int i = 0; i < size; i++) {
        sprintf((char*)&h,"%02x",buffer[i]);
        strcpy(hex + i * 2, h);
    }
    return hex;
}

char * bytes_to_hexdump(final byte *buffer, final int size) {
    if ( buffer == null ) {
        return null;
    } else {
        char *hexa_collector = null, *hexa_collector_tmp = null, *ascii_collector = null, *ascii_collector_tmp = null;
        char *result = null,*result_tmp = null;
        final int col_size = 20;
        int byte_i = 0;
        do {
            for(int col = 0; col < col_size && byte_i < size; col++, byte_i++) {
                final int byte_as_int = 0xFF&((int)buffer[byte_i]);
                if ( hexa_collector == null || col == 0 ) {
                    asprintf(&hexa_collector_tmp,"%02x", byte_as_int);
                } else {
                    asprintf(&hexa_collector_tmp,"%s %02x", hexa_collector, byte_as_int);
                    free(hexa_collector);
                }
                if ( 0x20 <= buffer[byte_i] && buffer[byte_i] <= 0x7E ) {
                    asprintf(&ascii_collector_tmp,"%s%c",ascii_collector==null?"":ascii_collector,buffer[byte_i]);
                } else {
                    asprintf(&ascii_collector_tmp,"%s.",ascii_collector==null?"":ascii_collector);
                }
                if ( ascii_collector != null ) {
                    free(ascii_collector);
                    ascii_collector = null;
                }
                ascii_collector = ascii_collector_tmp;
                hexa_collector = hexa_collector_tmp;
                hexa_collector_tmp = null;
                ascii_collector_tmp = null;
            }
            assert(0 < col_size);
            asprintf(&result_tmp,"%s%59s | %20s\n", result == null ? "" : result, hexa_collector==null?"NULL buffer":hexa_collector, ascii_collector==null?"":ascii_collector);
            if ( hexa_collector != null ) {
                free(hexa_collector);
                hexa_collector = null;
            }
            if ( ascii_collector != null ) {
                free(ascii_collector);
                ascii_collector = null;
            }
            if ( result != null ) {
                free(result);
            }
            result = result_tmp;
        } while(byte_i < size);
        return result;
    }
}

void bytes_dump(final byte *buffer, final int size) {
    if ( buffer == null ) {
        log_msg(LOG_DEBUG, "NULL buffer");
    } else {
        final char * result = bytes_to_hexdump(buffer, size);
        int sz = strlen(result);
        assert(0 < sz);
        if ( result[sz-1] == '\n' ) {
            result[sz-1] = 0;
        }
        log_msg(LOG_DEBUG, result);
        free(result);
    }
}