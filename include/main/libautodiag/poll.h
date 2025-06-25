#ifndef __CUSTOM_POLL_H
#define __CUSTOM_POLL_H

#include "libautodiag/compile_target.h"
#include "libautodiag/lang/lang.h"
#include "libautodiag/log.h"
#include <unistd.h>
#include <errno.h>

#ifdef OS_WINDOWS
#   include "libautodiag/windows.h"
#elif defined OS_POSIX
#   include <sys/ioctl.h>
#   include <poll.h>
    typedef struct pollfd POLLFD;
#endif

/**
 * All is a file implementation of the pool, support both windows and unix.
 * @return -1 on error, 0 on time out, 1 or greater on success
 */
int file_pool(void *handle, int *readLen, int timeout_ms);
int file_pool_write(void *handle, int timeout_ms);

#endif
