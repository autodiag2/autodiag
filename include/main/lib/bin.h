#ifndef __BIN_H
#define __BIN_H

#define _GNU_SOURCE
#include <stdio.h>
#include "lib/string.h"
#include <assert.h>
#include <stdlib.h>
#include "lang.h"
#include "log.h"

void bin_dump(final byte *buffer, final int size);
/**
 * Convert byte buffer to hexdump format
 */
char * buffer_to_hexdump(final byte *buffer, final int size);

#endif
