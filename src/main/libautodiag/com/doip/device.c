#include "libautodiag/com/doip/device.h"

static int doip_send(final object_DoIPDevice * device, const char * command) {
    // TODO : find an alternative with buffers
    return 0;
}
static int doip_recv(final object_DoIPDevice * device) {
    int readLen = DEVICE_ERROR;
    #if defined OS_POSIX
        if (device->implementation->handle < 0) {
            return DEVICE_ERROR;
        } else {
            int res = file_pool_read_posix(device->implementation->handle, &readLen, device->timeout);
            if ( 0 < res ) {
                buffer_ensure_capacity(device->recv_buffer, readLen);
                final int bytes_readed = read(device->implementation->handle, device->recv_buffer->buffer + device->recv_buffer->size, readLen);
                if( 0 < bytes_readed ) {
                    device->recv_buffer->size += bytes_readed;
                } else if ( bytes_readed == 0 ) {
                    log_msg(LOG_ERROR, "error during reception should read %d bytes", readLen);
                } else {
                    perror("read");
                    return DEVICE_ERROR;
                }
                if ( log_has_level(LOG_DEBUG) ) {
                    log_msg(LOG_DEBUG, "ip data received");
                    buffer_dump(device->recv_buffer);
                }
            } else if ( res == -1 ) {
                perror("poll");
            } else if ( res == 0 ) {
                log_msg(LOG_DEBUG, "Timeout while reading data");
            } else {
                log_msg(LOG_ERROR, "unexpected happen on serial line");
            }
        }
    #elif defined OS_WINDOWS
        DWORD bytes_readed = 0;

        int res = file_pool_read(&device->implementation->win_handle, &readLen, device->timeout);

        if ( res == -1 ) {
            log_msg(LOG_ERROR, "Error while polling");
        } else if ( res == 0 ) {
            log_msg(LOG_WARNING, "Timeout while polling");
        }
        
        if (isSocketHandle(device->implementation->win_handle)) {
            buffer_ensure_capacity(device->recv_buffer, readLen);
            SOCKET s = (SOCKET)device->implementation->win_handle;
            int r = recv(
                s,
                (char *)device->recv_buffer->buffer + device->recv_buffer->size,
                readLen,
                0
            );

            if (r > 0) {
                device->recv_buffer->size += r;
            } else {
                log_msg(LOG_ERROR, "recv failed: %d", WSAGetLastError());
                return DEVICE_ERROR;
            }
        } else {
            log_msg(LOG_ERROR, "cannot proceed on current handler");
        }
    #endif
    return readLen;
}
object_DoIPDevice * object_DoIPDevice_new() {
    object_DoIPDevice * device = (object_DoIPDevice*)malloc(sizeof(object_DoIPDevice));
    device->type = strdup(DOIP_DEVICE_TYPE);
    device->implementation = (DoIPDeviceImplementation*)malloc(sizeof(DoIPDeviceImplementation));
    #if defined OS_WINDOWS
        device->implementation->win_handle = INVALID_HANDLE_VALUE;
    #endif
    #if defined OS_POSIX
        device->implementation->handle = -1;
    #endif
    device->send = AD_DEVICE_SEND(doip_send);
    device->recv = AD_DEVICE_RECV(doip_recv);
    return device;
}

void object_DoIPDevice_free(object_DoIPDevice *device) {
    free(device);
}

object_DoIPDevice * object_DoIPDevice_assign(object_DoIPDevice * to, object_DoIPDevice * from) {
    memcpy(to, from, sizeof(object_DoIPDevice));
    return to;
}