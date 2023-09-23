#include "globals.h"
#include "log.h"

void unsupported_configuration() {
   log_msg("Unsupported configuration", LOG_INFO);
}

unsigned char byte_reverse(unsigned char b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    return (b & 0xAA) >> 1 | (b & 0x55) << 1;
}

char * buffer_to_hexdump(final char *buffer, final int size) {
    if ( buffer == null ) {
        return null;
    } else {
        char *hexa_collector = null, *hexa_collector_tmp = null, *ascii_collector = null, *ascii_collector_tmp = null;
        char *result = null,*result_tmp = null;
        final int cols = 20;
        int byte = 0;
        do {
            for(int col = 0; col < cols && byte < size; col++, byte++) {
                final int byte_as_int = 0xFF&((int)buffer[byte]);
                if ( hexa_collector == null || col == 0 ) {
                    asprintf(&hexa_collector_tmp,"%02x", byte_as_int);
                } else {
                    asprintf(&hexa_collector_tmp,"%s %02x", hexa_collector, byte_as_int);
                    free(hexa_collector);
                }
                if ( 0x20 <= buffer[byte] && buffer[byte] <= 0x7E ) {
                    asprintf(&ascii_collector_tmp,"%s%c",ascii_collector==null?"":ascii_collector,buffer[byte]);
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
            assert(0 < cols);
            asprintf(&result_tmp,"%s%59s | %20s\n", result==null?"":result, hexa_collector==null?"NULL buffer":hexa_collector, ascii_collector==null?"":ascii_collector);
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
        } while(byte < size);
        return result;
    }
}

void bin_dump(final char *buffer, final int size) {
    if ( buffer == null ) {
        log_msg("NULL buffer", LOG_DEBUG);
    } else {
        final char * result = buffer_to_hexdump(buffer, size);
        log_msg(result, LOG_DEBUG);
        free(result);
    }
}
