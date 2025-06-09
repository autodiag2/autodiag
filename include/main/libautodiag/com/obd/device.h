#ifndef __DEVICE_H
#define __DEVICE_H

#include "vehicle.h"

typedef struct {

} _Device;

/**
 * Define an OBD device
 */
typedef struct {
    _Device;
    /**
     * Send a string with the device
     * @return number of bytes sent or DEVICE_SEND_ERROR on error
     */
    int (*send)(final _Device* device, const char *command);
    /**
     * Receive data with the device
     * @return implementation dependent value DEVICE_RECV_ERROR on error
     */
    int  (*recv)(final _Device* device);
    /**
     * Open the device for sending and receiving
     */
    void (*open)(final _Device* device);
    /**
     * Close the device for sending and receiving it may be reopenned later
     */
    void (*close)(final _Device* device);
    /**
     * Get a string describing the underlying communication layer
     * eg "ISO 15765-4 (CAN 11-bit ID, 500 kBit/s)"
     */
    char* (*describe_communication_layer)(final _Device* device);
    /**
     * Parse a previously received data
     * eg OBD data
     * @return true if the parsing has succeed else false
     */
    bool (*parse_data)(final _Device* device, final Vehicle* vehicle);
    /**
     * Clear any previously received data on the device
     */
    void (*clear_data)(final _Device* device);
    /**
     * Acquire the lock on the device, prevent concurrent access by other threads
     */
    void (*lock)(final _Device* device);
    /**
     * Release the lock on the device by this thread
     */
    void (*unlock)(final _Device* device);
} Device;

#define DEVICE_ERROR -1
/**
 * OBD data received by the device
 */
#define DEVICE_RECV_DATA -2
/**
 * Nothing has been received by the device
 */
#define DEVICE_RECV_NULL -3
/**
 * When the request information is no available on the corresponding vehicle
 * The wanted OBD data is not available
 */
#define DEVICE_RECV_DATA_UNAVAILABLE -4

#define DEVICE(var) ((_Device*)var)

#endif
