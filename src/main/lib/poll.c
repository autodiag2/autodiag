#include "lib/poll.h"


int file_pool(void *handle, int *readLen, int timeout_ms) {
    #if defined OS_WINDOWS
        *readLen = 0;
        HANDLE connection_handle = (HANDLE)*((HANDLE*)handle);
        if (connection_handle == INVALID_HANDLE_VALUE) {
            return -1;
        } else {
            int sleep_length_ms = 20;
            if ( isComport(connection_handle) ) {
                for(int i = 0; i < timeout_ms / sleep_length_ms && *readLen == 0; i++) {
                    DWORD errors;
                    COMSTAT stat = {0};
                    ClearCommError(connection_handle, &errors, &stat);
                    readLen = stat.cbInQue;
                    if ( readLen == 0 ) {
                        usleep(1000 * sleep_length_ms);
                    }
                }
            } else {
                for(int i = 0; i < timeout_ms / sleep_length_ms && *readLen == 0; i++) {
                    PeekNamedPipe(connection_handle, NULL, 0, NULL, readLen, NULL);
                    if ( readLen == 0 ) {
                        usleep(1000 * sleep_length_ms);
                    }
                }
            }
        }
        return 0;
    #elif defined OS_POSIX
        final POLLFD fileDescriptor = {
            .fd = *((int*)handle),
            .events = POLLIN
        };
        return poll(&fileDescriptor,1,timeout_ms);
    #endif
}