#include "libautodiag/com/doip/device.h"

static int doip_send(final object_DoIPDevice * device, const char * command) {
    Buffer * request = buffer_from_ascii_hex(command);
    if ( request == null ) {
        request = buffer_from_ascii(command);
    }
    if ( log_has_level(LOG_DEBUG) ) {
        log_msg(LOG_DEBUG, "Sending");
        buffer_dump(request);
    }
    #ifdef OS_POSIX
        if (device->implementation->handle < 0) {
            return DEVICE_ERROR;
        }
    #endif
    #ifdef OS_WINDOWS
        #ifdef OS_POSIX
            else
        #endif
        if (device->implementation->win_handle == INVALID_HANDLE_VALUE) {
            return DEVICE_ERROR;
        }
    #endif
    int bytes_sent = 0;
    int write_len_rv = 0;
    int poll_result = -1;
    #ifdef OS_POSIX
        if ( device->implementation->handle != -1 ) {
            poll_result = file_pool_write_posix(device->implementation->handle, device->timeout);
        }
    #endif
    #ifdef OS_WINDOWS
        if ( device->implementation->win_handle != INVALID_HANDLE_VALUE ) {
            poll_result = file_pool_write(&device->implementation->win_handle, device->timeout);
        }
    #endif
    if ( poll_result == -1 ) {
        log_msg(LOG_ERROR, "Error while polling");
        return DEVICE_ERROR;
    } else if ( poll_result == 0 ) {
        log_msg(LOG_ERROR, "Timeout while polling for write");
        return 0;
    }
    #ifdef OS_POSIX
        if ( device->implementation->handle != -1 ) {
            bytes_sent = write(device->implementation->handle,request->buffer,request->size);
            if ( bytes_sent != request->size ) {
                perror("device write");
                log_msg(LOG_ERROR, "Error while writting to the doip");
                return DEVICE_ERROR;
            }
            return bytes_sent;
        }
    #endif

    #if defined OS_WINDOWS
        DWORD bytes_written;

        if (isSocketHandle(device->implementation->win_handle)) {
            SOCKET s = (SOCKET)device->implementation->win_handle;

            int sent = send(s, (const char *)request->buffer, request->size, 0);
            if (sent <= 0 || sent != request->size) {
                log_msg(LOG_ERROR, "send failed: %d", WSAGetLastError());
                return DEVICE_ERROR;
            }
            bytes_sent = sent;
        }
        if (bytes_sent != request->size) {
            log_msg(LOG_ERROR, "Error while writting to the doip");
            return DEVICE_ERROR;
        }
    #endif
    return bytes_sent;
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
                log_msg(LOG_ERROR, "unexpected happen on doip line");
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
    device->location = null;
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