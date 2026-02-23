#ifndef __COM_DOIP_DEVICE_H
#define __COM_DOIP_DEVICE_H

#include "libautodiag/lib.h"
#include "libautodiag/com/device.h"
#if defined OS_POSIX
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <unistd.h>
#   include <errno.h>
#endif
#if defined OS_WINDOWS
#   include "libautodiag/windows.h"
#elif defined OS_POSIX

#else
#   warning Unsupported OS
#endif

#define DOIP_DEVICE_TYPE "doip"

typedef struct {
    pthread_mutex_t lock_mutex;    // thread lock on the port (both buffer and other data)
#if defined OS_WINDOWS
    HANDLE win_handle;
    #ifdef OS_POSIX
        int handle;
    #endif
#elif defined OS_POSIX
    int handle;
#else
#   warning Unsupported OS
#endif
} DoIPDeviceImplementation;

typedef enum {
    DEVICE_DOIP_STATUS_OPEN,
    DEVICE_DOIP_STATUS_NOT_OPEN,
    DEVICE_DOIP_STATUS_ERROR
} DeviceDoIPStatus;

OBJECT_H(DoIPDevice,
    Device;
    DoIPDeviceImplementation * implementation;
    int timeout;                // timeout in ms before considering no reply from the remote
    Buffer * recv_buffer;       // buffer for input data
    DeviceDoIPStatus status;    // state of the device
);
/**
 * Use this to send/recv anything else other than diag messages.
 */
int doip_send_internal(final object_DoIPDevice * device, const char * command);
int doip_recv_internal(final object_DoIPDevice * device);
void doip_close(final object_DoIPDevice * device);
/**
 * Configure the device so it is up and ready for sending and receiving diag messages.
 */
bool doip_configure(final object_DoIPDevice * device);

#endif