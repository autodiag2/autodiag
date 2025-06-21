#ifndef __DEVICE_H
#define __DEVICE_H

#include "libautodiag/model/vehicle.h"

/**
 * Define a device that interface with the vehicle
 */
#define CAST_DEVICE_SEND(var) ((int (*)(final Device*, const char*))var)
#define CAST_DEVICE_RECV(var) ((int (*)(final Device*))var)
#define CAST_DEVICE_OPEN(var) ((void (*)(final Device*))var)
#define CAST_DEVICE_CLOSE(var) ((void (*)(final Device*))var)
#define CAST_DEVICE_DESCRIBE_COMMUNICATION_LAYER(var) ((char* (*)(final Device*))var)
#define CAST_DEVICE_PARSE_DATA(var) ((bool (*)(final Device*, final Vehicle*))var)
#define CAST_DEVICE_CLEAR_DATA(var) ((void (*)(final Device*))var)
#define CAST_DEVICE_LOCK(var) ((void (*)(final Device*))var)
#define CAST_DEVICE_UNLOCK(var) ((void (*)(final Device*))var)
#define CAST_DEVICE(arg) ((struct Device*)arg)
typedef struct Device {
    /**
     * Send a string with the device
     * @return number of bytes sent or DEVICE_SEND_ERROR on error
     */
    int (*send)(final struct Device* device, const char *command);
    /**
     * Receive data with the device
     * @return implementation dependent value DEVICE_RECV_ERROR on error
     */
    int  (*recv)(final struct Device* device);
    /**
     * Open the device for sending and receiving
     */
    void (*open)(final struct Device* device);
    /**
     * Close the device for sending and receiving it may be reopenned later
     */
    void (*close)(final struct Device* device);
    /**
     * Get a string describing the underlying communication layer
     * eg "ISO 15765-4 (CAN 11-bit ID, 500 kBit/s)"
     */
    char* (*describe_communication_layer)(final struct Device* device);
    /**
     * Parse a previously received data
     * eg OBD data
     * @return true if the parsing has succeed else false
     */
    bool (*parse_data)(final struct Device* device, final Vehicle* vehicle);
    /**
     * Clear any previously received data on the device
     */
    void (*clear_data)(final struct Device* device);
    /**
     * Acquire the lock on the device, prevent concurrent access by other threads
     */
    void (*lock)(final struct Device* device);
    /**
     * Release the lock on the device by this thread
     */
    void (*unlock)(final struct Device* device);
} Device;

#define DEVICE_ERROR -1
/**
 * Binary data received by the device
 * eg OBD Data hex string on the serial device
 */
#define DEVICE_RECV_DATA -2
/**
 * Nothing has been received by the device
 */
#define DEVICE_RECV_NULL -3
/**
 * When the request information is no available on the corresponding vehicle
 * eg The wanted OBD data is not available
 */
#define DEVICE_RECV_DATA_UNAVAILABLE -4

#endif
