#ifndef __AD_COM_DOIP_DEVICE_H
#define __AD_COM_DOIP_DEVICE_H

#include "libautodiag/lib.h"
#include "libautodiag/com/device.h"
#include "libautodiag/com/network.h"
#if defined OS_POSIX
#   include <sys/types.h>
#   include <unistd.h>
#   include <errno.h>
#endif
#if defined OS_WINDOWS
#   include "libautodiag/windows.h"
#elif defined OS_POSIX

#else
#   warning Unsupported OS
#endif
#include "libautodiag/com/doip/doip.h"

typedef struct {
    /**
     * thread lock on the port (both buffer and other data)
     */
    pthread_mutex_t lock_mutex;
    /**
     * Holds the connection (TCP) to the DoIP node, it is used for sending and receiving data.
     */
    sock_t handle;
    /**
     * Holds the UDP connection to the doip node.
     */
    sock_t disc_handle;
} DoIPDeviceImplementation;

#define DEVICE_DOIP_DEFAULT_ADDRESS 0x0E08
// General timeout
#define DEVICE_DOIP_DEFAULT_TIMEOUT_MS 1000
// A_DoIP_Ctrl_ms
#define DEVICE_DOIP_DEFAULT_TIMEOUT_CTRL_MS 2000
AD_OBJECT_H(DoIPDevice,
    Device;
    DoIPDeviceImplementation * implementation;
    // timeout in ms before considering no reply from the remote
    int timeout_ms;
    /**
     * Maximum time to wait for the doip entity (the device) for a previously sent UDP message (eg power mode request).
     */
    int A_DoIP_Ctrl_ms;
    // Address of the device on the DoIP network (tester address)
    uint16_t address;
    struct {
        int node_type;
        int max_data_size;
        int max_concurrent_connections;
        Buffer * address;
    } node;
);
/**
 * Use this to send/recv anything else other than diag messages.
 */
int doip_send_internal(final ad_object_DoIPDevice * device, const char * command);
int doip_recv_internal(final ad_object_DoIPDevice * device);
void doip_close(final ad_object_DoIPDevice * device);
/**
 * Configure the device so it is up and ready for sending and receiving diag messages.
 */
bool doip_configure(final ad_object_DoIPDevice * device);
/**
 * Set the tester address on the DoIP network, default is 0x0E08 but some vehicles may expect another one
 */
void doip_set_tester_address(final ad_object_DoIPDevice* device, uint16_t address);
/**
 * @return true if the node queue is full and cannot accept new connections else false or bool_unset if an error happened during the check
 */
bool doip_node_queue_is_full(final ad_object_DoIPDevice * device);
/**
 * Open the doip device.
 */
int doip_open(final ad_object_DoIPDevice * device);
/**
 * Receive over UDP a message
 */
ad_object_DoIPMessage *doip_disc_recv(ad_object_DoIPDevice *device);
/**
 * Send a message over UDP
 */
int doip_disc_send(ad_object_DoIPDevice *device, ad_object_DoIPMessage *msg);

#endif