#include "com/serial/serial.h"
#include <stdio.h>
#include <assert.h>

void serial_strip_char(final Buffer * buffer, final char * char_to_strip);

void test_comport_init() {
    Serial serial;
    assert(serial.number == 0);
}

void test_strip_char() {
    final BUFFER b = buffer_new();
    buffer_ensure_capacity(b, 100);
    strcpy(b->buffer,"\r\r>");
    b->size_used = 4;
    serial_strip_char(b,"\r");
    assert(strlen(b->buffer) == 1);
}

int main() {
    test_comport_init();
    test_strip_char();
	return 0;
}
