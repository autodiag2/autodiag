

#define _GNU_SOURCE
#include <stdio.h>
#include "libautodiag/string.h"
#include <assert.h>
#include <stdlib.h>
#include "lang.h"
#include "log.h"

void bytes_dump(final byte *buffer, final int size);
/**
 * Convert byte buffer to hexdump format
 */
char * bytes_to_hexdump(final byte *buffer, final int size);

char * bytes_to_ascii(final byte *buffer, final int size);
char * bytes_to_hex_string(final byte *buffer, final int size);