#ifndef __LIBAUTODIAG_CONFIG_H
#define __LIBAUTODIAG_CONFIG_H

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "lang.h"
#include "compile_target.h"

/**
 * Get the storage directory
 */
char *installation_folder(char *relative_path);

#endif