#include "libTest.h"
#include "libautodiag/buffer.h"

void testLeftShift() {
    {
        final Buffer * b = buffer_from_ascii_hex("0000");
        buffer_left_shift(b,2);
        assert(b->size == 0);
    }
    {
        final Buffer * b = buffer_from_ascii_hex("000102");
        buffer_left_shift(b,2);
        assert(b->size == 1);
        assert(b->buffer[0] == 0x02);
    }
}

bool testBuffer() {
    {
        Buffer * buffer = buffer_new();
        buffer_ensure_capacity(buffer,3);
        buffer->buffer[0] = 0x00;
        buffer->buffer[1] = 0x01;
        buffer->buffer[2] = 0x02;
        buffer->size = 3;
        char* ascii = buffer_to_hex_string(buffer);
        testOutput(ascii);
        assert(strcmp(ascii,"000102")==0);
    }
    {
        final Buffer * b = buffer_new();
        buffer_ensure_capacity(b, 100);
        strcpy(b->buffer,"\r\r>");
        b->size = 4;
        serial_strip_char(b,"\r");
        assert(strlen(b->buffer) == 1);
    }
    testLeftShift();
    {
        assert(buffer_new() != null);
    }
    {
        final Buffer * b = buffer_from_ascii_hex("0000000");    
        assert(buffer_ensure_capacity(b, 4));
        assert(!buffer_ensure_capacity(b, 4));
    }
    {
        final Buffer * b = buffer_from_ascii_hex("0000000");
        Buffer * b1 = buffer_new();
        buffer_append(b1,b);
        assert(b->size == b1->size);    
    }
    {
        {
            final Buffer * b = buffer_from_ascii_hex("010203");    
            buffer_ensure_termination(b);
            assert(b->size == 4);
        }
        {
            final Buffer * b = buffer_from_ascii_hex("010200");    
            buffer_ensure_termination(b);
            assert(b->size == 3);
        }
    }
    {
        assert(buffer_alphabet_compare("AA", "A0", "AZ"));
        assert(!buffer_alphabet_compare("BZ", "A0", "AZ"));
        assert(buffer_alphabet_compare("A0", "A0", "AZ"));
        assert(buffer_alphabet_compare("AZ", "A0", "AZ"));
    }
    {
        assert(buffer_from_ascii_hex("Z123") == null);
        assert(buffer_from_ascii_hex("G123") == null);
        assert(buffer_from_ascii_hex("12G3") == null);
        assert(buffer_from_ascii_hex("1123") != null);
    }
    return true;
}
