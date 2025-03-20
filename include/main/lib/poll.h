#ifndef __CUSTOM_POLL_H
#define __CUSTOM_POLL_H

#include "compile_target.h"
#include "lang.h"

#ifdef OS_WINDOWS
#   include <windows.h>
    bool isComport(HANDLE file);
#elif defined OS_POSIX
#   include <poll.h>
    typedef struct pollfd POLLFD;
#endif

/**
 * All is a file implementation of the pool, support both windows and unix.
 * no readLen support on unix.
 * @return -1 on error, positive value depends on the implementation
 */
int file_pool(void *handle, int *readLen, int timeout_ms);

#endif
