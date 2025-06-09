#include "libTest.h"
#include "lib/lib.h"

bool testGlobals() {
    {
        assert(byte_reverse(0xF0) == 0x0F);
        assert(byte_reverse(0x80) == 0x01);
        assert(byte_reverse(0x01) == 0x80);
        assert(byte_reverse(0x10) == 0x08);
    }
    {
        BUFFER buffer = buffer_from_ascii_hex("000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F20");
        final char * result = bytes_to_hexdump(buffer->buffer, buffer->size);
        buffer_ensure_termination(buffer);
        char * ptr = strstr(result,"\n");
        int c = (void*)ptr - (void*)result;
        testOutput(result);
        assert(c >= 20);
        assert(strlen(ptr) >= 20);
    }
    return true;
}
