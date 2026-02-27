#ifndef __HANDLE_H
#define __HANDLE_H

#include "libautodiag/compile_target.h"
#include "libautodiag/lang/all.h"
#include "libautodiag/log.h"

#if defined OS_WINDOWS
#   include "libautodiag/windows.h"
#elif defined OS_POSIX
#   include <termios.h>
#else
#   warning Unsupported OS
#endif

/**
 * OS indépendant resource locator.
 * file for posix
 * pseudo tty, socket, named pipe for windows
 */
OBJECT_H(handle_t,
    #ifdef OS_POSIX
        int posix_handle;
    #endif
    #ifdef OS_WINDOWS
        HANDLE win_handle;
        SOCKET win_socket;
    #endif
);

/**
 * Test if a handle is unset (invalid)
 */
bool object_handle_t_invalid(object_handle_t * h);
/**
 * Is the handle ready for data writting.
 * @return see poll
 */
int object_handle_t_poll_write(object_handle_t * h, int timeout_ms);
/**
 * Write some data to the handle.
 * @return -1 on error, bytes_written on success
 */
int object_handle_t_write(object_handle_t * h, int timeout_ms, byte * tx_buf, int bytes_to_send);
/**
 * Is the handle ready for data reading.
 * @return see poll
 */
int object_handle_t_poll_read(object_handle_t * h, int *readLen_rv, int timeout_ms);

#endif