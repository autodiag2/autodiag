#include "libautodiag/poll.h"

#if defined OS_WINDOWS
    bool isComport(HANDLE file) {
        DCB dcb;
        return GetCommState(file,&dcb);
    }
#endif

int file_pool(void *handle, int *readLen_rv, int timeout_ms) {
    #if defined OS_WINDOWS
        int readLen = 0;
        HANDLE connection_handle = (HANDLE)*((HANDLE*)handle);
        if (connection_handle == INVALID_HANDLE_VALUE) {
            return -1;
        } else {
            int sleep_length_ms = 20;
            if ( isComport(connection_handle) ) {
                for(int i = 0; i < timeout_ms / sleep_length_ms && readLen == 0; i++) {
                    DWORD errors;
                    COMSTAT stat = {0};
                    ClearCommError(connection_handle, &errors, &stat);
                    readLen = stat.cbInQue;
                    if ( readLen == 0 ) {
                        usleep(1000 * sleep_length_ms);
                    }
                }
            } else {
                for(int i = 0; i < timeout_ms / sleep_length_ms && readLen == 0; i++) {
                    PeekNamedPipe(connection_handle, NULL, 0, NULL, &readLen, NULL);
                    if ( readLen == 0 ) {
                        usleep(1000 * sleep_length_ms);
                    }
                }
            }
        }
        if ( readLen_rv != null ) {
            *readLen_rv = readLen;
        }
        return 0;
    #elif defined OS_POSIX
        struct pollfd fileDescriptor = {
            .fd = *((int*)handle),
            .events = POLLIN
        };
        int ret = poll(&fileDescriptor, 1, timeout_ms);
        if (ret > 0 && (fileDescriptor.revents & POLLIN)) {
            if (readLen_rv != null) {
                int available = 0;
                if (ioctl(fileDescriptor.fd, FIONREAD, &available) == 0) {
                    *readLen_rv = available;
                } else {
                    *readLen_rv = 0;
                }
            }
        } else if (readLen_rv != null) {
            *readLen_rv = 0;
        }
        return ret;
    #endif
}