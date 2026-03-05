#include "libautodiag/handle.h"

AD_OBJECT_SRC(handle_t)

int object_handle_t_get_port(object_handle_t * handle) {
    #ifdef OS_POSIX
        return sock_t_get_port(handle->posix_handle);
    #elif defined OS_WINDOWS
        return sock_t_get_port(handle->win_socket);
    #else
    #   warning Unsupported OS
        return -1;
    #endif
}
void object_handle_t_init(object_handle_t * h) {
    #ifdef OS_POSIX
        h->posix_handle = -1;
    #endif
    #ifdef OS_WINDOWS
        h->win_handle = INVALID_HANDLE_VALUE;
        #ifndef OS_POSIX
            h->win_socket = INVALID_SOCKET;
        #endif
    #endif
}
object_handle_t * object_handle_t_new() {
    object_handle_t * h = (object_handle_t*)malloc(sizeof(object_handle_t));
    object_handle_t_init(h);
    return h;
}
void object_handle_t_free(object_handle_t * h) {
    if ( h != null ) {
        free(h);
    }
}
object_handle_t * object_handle_t_assign(object_handle_t * to, object_handle_t * from) {
    #ifdef OS_POSIX
        to->posix_handle = from->posix_handle;
    #endif
    #ifdef OS_WINDOWS
        to->win_handle = from->win_handle;
        #ifndef OS_POSIX
            to->win_socket = from->win_socket;
        #endif
    #endif
    return to;
}
bool object_handle_t_invalid(object_handle_t * h) {
    bool result = true;
    #ifdef OS_POSIX
        if ( h->posix_handle != -1 ) {
            result = false;
        }
    #endif
    #ifdef OS_WINDOWS
        if ( h->win_handle != INVALID_HANDLE_VALUE ) {
            result = false;
        }
        #ifndef OS_POSIX
            if ( h->win_socket != INVALID_SOCKET ) {
                result = false;
            }
        #endif
    #endif
    return result;
}
int object_handle_t_read(object_handle_t * h, byte * dst, int size) {
    int bytes_readed = -1;
    #ifdef OS_POSIX
        if ( h->posix_handle != -1 ) {
            return read(h->posix_handle, dst, size);
        }
    #endif
    #ifdef OS_WINDOWS
        if ( h->win_handle != INVALID_HANDLE_VALUE ) {
            DWORD NumberOfBytesRead;
            if ( ! ReadFile(h->win_handle, dst, size, &NumberOfBytesRead, 0) ) {
                log_msg(LOG_ERROR, "ReadFile error 2");
            }
            return (int)NumberOfBytesRead;
        }
        #ifndef OS_POSIX 
            if ( h->win_socket != INVALID_SOCKET ) {
                int bytes_readed = recv(
                    h->win_socket,
                    (char *)dst,
                    size,
                    0
                );

                if (bytes_readed <= 0) {
                    log_msg(LOG_ERROR, "recv failed: %d", WSAGetLastError());
                    return -1;
                }

                return bytes_readed;
            }
        #endif
    #endif
    return -1;
}
int object_handle_t_poll_read(object_handle_t * h, int *readLen_rv, int timeout_ms) {
    if ( readLen_rv != null ) {
        *readLen_rv = 0;
    }
    if ( object_handle_t_invalid(h) ) {
        return -1;
    }
    #if defined OS_WINDOWS
        int sleep_length_ms = 20;
        final int max_tries = timeout_ms / sleep_length_ms ;
        int tries = 0;
        DWORD readLen = 0;
        bool handleAvailable = false;
        if ( h->win_handle != INVALID_HANDLE_VALUE ) {
            if ( isComPort(h->win_handle) ) {
                for(tries = 0; tries < max_tries && readLen == 0; tries++) {
                    DWORD errors;
                    COMSTAT stat = {0};
                    ClearCommError(h->win_handle, &errors, &stat);
                    readLen = stat.cbInQue;
                    if ( readLen == 0 ) {
                        usleep(1000 * sleep_length_ms);
                    }
                }
            } else {
                for(tries = 0; tries < max_tries && readLen == 0; tries++) {
                    PeekNamedPipe(h->win_handle, NULL, 0, NULL, &readLen, NULL);
                    if ( readLen == 0 ) {
                        usleep(1000 * sleep_length_ms);
                    }
                }
            }
            if ( tries == max_tries) {
                return 0;
            }
            handleAvailable = true;
        }
        #ifndef OS_POSIX 
        else if ( h->win_socket != INVALID_SOCKET ) {
                
            WSAPOLLFD pfd = {
                .fd = h->win_socket,
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
            if (ioctlsocket(h->win_socket, FIONREAD, &avail) != 0) {
                return -1;
            }
            readLen = avail;
            handleAvailable = true;
        }
        #endif
    
        if ( handleAvailable ) {
            if ( readLen_rv != null ) {
                if ( INT_MAX < readLen ) {
                    log_msg(LOG_WARNING, "The size readed is more than the capacity of implementation, restricting to ");
                    *readLen_rv = INT_MAX;
                } else {
                    *readLen_rv = readLen;
                }
            }
            return 1;
        }
    #endif
    #if defined OS_POSIX
        return file_pool_read_posix(h->posix_handle, readLen_rv, timeout_ms);
    #endif
    return -1;
}
int object_handle_t_poll_write(object_handle_t * h, int timeout_ms) {
    int poll_result = -1;
    #ifdef OS_POSIX
        if ( h->posix_handle != -1 ) {
            poll_result = file_pool_write_posix(h->posix_handle, timeout_ms);
        }
    #endif
    #ifdef OS_WINDOWS
        int sleep_length_ms = 20;
        final int max_tries = timeout_ms / sleep_length_ms ;
        int tries = 0;
        if ( h->win_handle != INVALID_HANDLE_VALUE ) {
            if (isComPort(h->win_handle)) {
                for(tries = 0; tries < max_tries; tries++) {
                    COMSTAT stat = {0};
                    DWORD errors = 0;
                    if ( ! ClearCommError(h->win_handle, &errors, &stat) ) {
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
        #ifndef OS_POSIX
            if ( h->win_socket != INVALID_SOCKET ) {
                WSAPOLLFD pfd = {
                    .fd = h->win_socket,
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
                return 0;
            }
        #endif
    #endif
    return poll_result;
}
int object_handle_t_write(object_handle_t * h, byte * tx_buf, int bytes_to_send) {

    if ( object_handle_t_invalid(h) ) {
        return -1;
    }

    #ifdef OS_POSIX
        if ( h->posix_handle != -1 ) {
            int bytes_sent = write(h->posix_handle, tx_buf, bytes_to_send);
            if ( bytes_sent != bytes_to_send ) {
                log_msg(LOG_ERROR, "Error while writting to the serial");
                return -1;
            }
            return bytes_sent;
        }
    #endif

    #if defined OS_WINDOWS
        DWORD bytes_written;

        if ( h->win_handle != INVALID_HANDLE_VALUE ) {
            if ( isComPort(h->win_handle) ) {
                PurgeComm(h->win_handle, PURGE_TXCLEAR|PURGE_RXCLEAR);
            }
            if (!WriteFile(h->win_handle, tx_buf, bytes_to_send, &bytes_written, null)) {
                log_msg(LOG_ERROR, "WriteFile failed with error %lu", GetLastError());
                return -1;
            }            
            return (int) bytes_written;
        }
        #ifndef OS_POSIX 
            if ( h->win_socket != INVALID_SOCKET ) {
                int sent = send(h->win_socket, (const char *)tx_buf, bytes_to_send, 0);
                if (sent <= 0 || sent != bytes_to_send) {
                    log_msg(LOG_ERROR, "send failed: %d", WSAGetLastError());
                    return -1;
                }
                return sent;
            }
        #endif
    #endif

    return -1;
}
void object_handle_t_close(object_handle_t * h) {
    #if defined OS_POSIX
        if (0 <= h->posix_handle) {
            shutdown(h->posix_handle, SHUT_RDWR);
            close(h->posix_handle);
            h->posix_handle = -1;
        }
    #endif
    #if defined OS_WINDOWS
        if ( h->win_handle != INVALID_HANDLE_VALUE ) {
            if ( isComPort(h->win_handle) ) {
                PurgeComm(h->win_handle, PURGE_TXCLEAR|PURGE_RXCLEAR);
            }
            CloseHandle(h->win_handle);
            h->win_handle = INVALID_HANDLE_VALUE;
        }
        #ifndef OS_POSIX
        if ( h->win_socket != INVALID_SOCKET ) {
            shutdown(h->win_socket, SD_BOTH);
            closesocket(h->win_socket);
            h->win_socket = INVALID_SOCKET;
        }
        #endif
    #endif
}