#include "libTest.h"
#include "libautodiag/buffer.h"

void testLeftShift() {
    {
        final Buffer * b = ad_buffer_from_ascii_hex("0000");
        ad_buffer_left_shift(b,2);
        assert(b->size == 0);
    }
    {
        final Buffer * b = ad_buffer_from_ascii_hex("000102");
        ad_buffer_left_shift(b,2);
        assert(b->size == 1);
        assert(b->buffer[0] == 0x02);
    }
}

bool testBuffer() {
    {
        Buffer * buffer = ad_buffer_new();
        ad_buffer_ensure_capacity(buffer,3);
        buffer->buffer[0] = 0x00;
        buffer->buffer[1] = 0x01;
        buffer->buffer[2] = 0x02;
        buffer->size = 3;
        char* ascii = ad_buffer_to_hex_string(buffer);
        tf_test_output(ascii);
        assert(strcmp(ascii,"000102")==0);
    }
    testLeftShift();
    {
        assert(ad_buffer_new() != null);
    }
    {
        final Buffer * b = ad_buffer_from_ascii_hex("0000000");    
        assert(ad_buffer_ensure_capacity(b, 4));
        assert(!ad_buffer_ensure_capacity(b, 4));
    }
    {
        final Buffer * b = ad_buffer_from_ascii_hex("0000000");
        Buffer * b1 = ad_buffer_new();
        ad_buffer_append(b1,b);
        assert(b->size == b1->size);    
    }
    {
        {
            final Buffer * b = ad_buffer_from_ascii_hex("010203");    
            ad_buffer_ensure_termination(b);
            assert(b->size == 4);
        }
        {
            final Buffer * b = ad_buffer_from_ascii_hex("010200");    
            ad_buffer_ensure_termination(b);
            assert(b->size == 3);
        }
    }
    {
        assert(ad_buffer_alphabet_compare("AA", "A0", "AZ"));
        assert(!ad_buffer_alphabet_compare("BZ", "A0", "AZ"));
        assert(ad_buffer_alphabet_compare("A0", "A0", "AZ"));
        assert(ad_buffer_alphabet_compare("AZ", "A0", "AZ"));
    }
    {
        assert(ad_buffer_from_ascii_hex("Z123") == null);
        assert(ad_buffer_from_ascii_hex("G123") == null);
        assert(ad_buffer_from_ascii_hex("12G3") == null);
        assert(ad_buffer_from_ascii_hex("1123") != null);
    }
    {
        Buffer * b = ad_buffer_from_ints(0x00, 0x01);
        assert(b != null);
        assert(b->size == 2);
        assert(b->buffer[0] == 0x00);
        assert(b->buffer[1] == 0x01);
    }
    return true;
}
