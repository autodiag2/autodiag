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

#define DEVICE_DOIP_DEFAULT_ADDRESS 0x0E08
#define DEVICE_DOIP_DEFAULT_TIMEOUT_MS 1000
OBJECT_H(DoIPDevice,
    Device;
    DoIPDeviceImplementation * implementation;
    // timeout in ms before considering no reply from the remote
    int timeout_ms;
    // buffer for input data
    Buffer * recv_buffer;
    // state of the device
    DeviceDoIPStatus status;
    // Address of the device on the DoIP network (tester address)
    uint16_t address;
    struct {
        int node_type;
        int max_data_size;
    } node;
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
/**
 * Set the tester address on the DoIP network, default is 0x0E08 but some vehicles may expect another one
 */
void doip_set_tester_address(final object_DoIPDevice* device, uint16_t address);
/**
 * @return true if the node queue is full and cannot accept new connections else false or bool_unset if an error happened during the check
 */
bool doip_node_queue_is_full(final object_DoIPDevice * device);

#endif