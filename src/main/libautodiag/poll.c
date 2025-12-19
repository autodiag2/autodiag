#include "libautodiag/poll.h"

int file_pool_read(void *handle, int *readLen_rv, int timeout_ms) {
    if ( readLen_rv != null ) {
        *readLen_rv = 0;
    }
    #if defined OS_WINDOWS
        DWORD readLen = 0;
        HANDLE connection_handle = (HANDLE)*((HANDLE*)handle);
        if (connection_handle == INVALID_HANDLE_VALUE) {
            return -1;
        }
        int sleep_length_ms = 20;
        final int max_tries = timeout_ms / sleep_length_ms ;
        int tries = 0;
        if ( isSocketHandle(connection_handle) ) {
            SOCKET s = (SOCKET)connection_handle;
            
            WSAPOLLFD pfd = {
                .fd = s,
                .events = POLLRDNORM
            };
            
            int res = WSAPoll(&pfd, 1, timeout_ms);
            if (res <= 0) {
                return res;
            }

            if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
                return -1;
            }

            u_long avail = 0;
            if (ioctlsocket(s, FIONREAD, &avail) != 0) {
                return -1;
            }
            readLen = avail;
        } else if ( isComPort(connection_handle) ) {
            for(tries = 0; tries < max_tries && readLen == 0; tries++) {
                DWORD errors;
                COMSTAT stat = {0};
                ClearCommError(connection_handle, &errors, &stat);
                readLen = stat.cbInQue;
                if ( readLen == 0 ) {
                    usleep(1000 * sleep_length_ms);
                }
            }
        } else {
            for(tries = 0; tries < max_tries && readLen == 0; tries++) {
                PeekNamedPipe(connection_handle, NULL, 0, NULL, &readLen, NULL);
                if ( readLen == 0 ) {
                    usleep(1000 * sleep_length_ms);
                }
            }
        }
        if ( tries == max_tries) {
            return 0;
        }
    
        if ( readLen_rv != null ) {
            if ( INT_MAX < readLen ) {
                log_msg(LOG_WARNING, "The size readed is more than the capacity of implementation, restricting to ");
                *readLen_rv = INT_MAX;
            } else {
                *readLen_rv = readLen;
            }
        }
        return 1;
    #elif defined OS_POSIX
        struct pollfd fileDescriptor = {
            .fd = *((int*)handle),
            .events = POLLIN
        };
        int ret = poll(&fileDescriptor, 1, timeout_ms);
        if ( ret > 0 ) {
            if ( fileDescriptor.revents & POLLIN ) {
                if (readLen_rv != null) {
                    int available = 0;
                    if (ioctl(fileDescriptor.fd, FIONREAD, &available) == 0) {
                        *readLen_rv = available;
                    } else {
                        *readLen_rv = 0;
                    }
                }
            } else {
                ret = -1;
            }
        }
        return ret;
    #endif
}

int file_pool_write(void *handle, int timeout_ms) {
    #if defined OS_WINDOWS
        HANDLE connection_handle = (HANDLE)*((HANDLE*)handle);
        if (connection_handle == INVALID_HANDLE_VALUE) {
            return -1;
        } else {
            int sleep_length_ms = 20;
            final int max_tries = timeout_ms / sleep_length_ms ;
            int tries = 0;
            if ( isSocketHandle(connection_handle) ) {
                SOCKET s = (SOCKET)connection_handle;

                WSAPOLLFD pfd = {
                    .fd = s,
                    .events = POLLWRNORM
                };

                int res = WSAPoll(&pfd, 1, timeout_ms);
                if (res <= 0) {
                    return res;
                }
                if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
                    return -1;
                }
                if (pfd.revents & POLLWRNORM) {
                    return res;
                }
            } else if (isComPort(connection_handle)) {
                for(tries = 0; tries < max_tries; tries++) {
                    COMSTAT stat = {0};
                    DWORD errors = 0;
                    if ( ! ClearCommError(connection_handle, &errors, &stat) ) {
                        return -1;
                    }
                    /* if no flow-control hold, port is writable */
                    if (!(stat.fCtsHold || stat.fDsrHold || stat.fRlsdHold || stat.fXoffHold) && stat.cbOutQue == 0) {
                        return 1;
                    }
                    usleep(1000 * sleep_length_ms);
                }
            } else {
                DWORD err = GetLastError();
                if ( err == ERROR_BROKEN_PIPE || err == ERROR_PIPE_NOT_CONNECTED ) {
                    return -1;
                }
                return 1;
            }
            return 0;
        }
    #elif defined OS_POSIX
        struct pollfd pfd = {
            .fd     = *((int*)handle),
            .events = POLLOUT
        };
        int ret = poll(&pfd, 1, timeout_ms);
        if (0 < ret) {
            if ( ! ( pfd.revents & POLLOUT ) ) {
                ret = -1;
            }
        }
        return ret;
    #endif
}