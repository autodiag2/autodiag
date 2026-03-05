#ifndef __DEVICE_H
#define __DEVICE_H

#include <ctype.h>
#include "libautodiag/model/vehicle.h"

/**
 * Define a device that interface with the vehicle
 */
#define AD_DEVICE_SEND(var) ((int (*)(final Device*, const char*))var)
#define AD_DEVICE_RECV(var) ((int (*)(final Device*))var)
#define AD_DEVICE_FREE(var) ((void (*)(final Device*))var)
#define AD_DEVICE_OPEN(var) ((int (*)(final Device*))var)
#define AD_DEVICE_CLOSE(var) ((void (*)(final Device*))var)
#define AD_DEVICE_DESCRIBE_COMMUNICATION_LAYER(var) ((char* (*)(final Device*))var)
#define AD_DEVICE_PARSE_DATA(var) ((bool (*)(final Device*, final Vehicle*))var)
#define AD_DEVICE_SET_FILTER_BY_ADDRESS(var) ((bool (*)(final Device*, final ad_list_Buffer*))var)
#define AD_DEVICE_CLEAR_DATA(var) ((void (*)(final Device*))var)
#define AD_DEVICE_LOCK(var) ((void (*)(final Device*))var)
#define AD_DEVICE_UNLOCK(var) ((void (*)(final Device*))var)
#define AD_DEVICE(arg) ((struct Device*)arg)
#define AD_DEVICE_DESCRIBE_STATE(var) ((const char* (*)(final Device*))var)

typedef enum {
    AD_DEVICE_TYPE_AUTO = 0,
    AD_DEVICE_TYPE_SERIAL = 1,
    AD_DEVICE_TYPE_DOIP = 2
} AD_DEVICE_TYPE;

typedef enum {
    AD_DEVICE_STATE_UNDEFINED = 0,
    AD_DEVICE_STATE_READY = 1,
    AD_DEVICE_STATE_NOT_READY = 2,
    AD_DEVICE_STATE_DEVICE_SPECIFIC = 3
} AD_DEVICE_STATE;

typedef struct Device {
    /**
     * eg. DoIP, Serial
     */
    AD_DEVICE_TYPE type;
    /**
     * State of the device
     */
    AD_DEVICE_STATE state;
    /**
     * buffer for input data
     */
    Buffer * recv_buffer;
    /**
     * location of the device accross OSs
     */
    char *location;
    /**
     * did the serial port has been detected during the previous scan
     */
    bool detected;
    /**
     * Send a string with the device
     * @return number of bytes sent or DEVICE_ERROR on error
     */
    int (*send)(final struct Device* device, const char *command);
    /**
     * Receive data with the device
     * @return implementation dependent value DEVICE_ERROR on error
     */
    int  (*recv)(final struct Device* device);
    /**
     * Open the device for sending and receiving
     * @returns GENERIC_FUNCTION_SUCCESS on success else GENERIC_FUNCTION_ERROR
     */
    int (*open)(final struct Device* device);
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
     * Get a string representing state of the device, with device specific information if the state is DEVICE_STATE_DEVICE_SPECIFIC
     * @return null if the error is unhandled.
     */
    const char* (*describe_state)(final struct Device* device);
    /**
     * Parse a previously received data and store in corresponding ECU in the vehicle
     * eg OBD data, UDS data
     * Extract higher layer information from the low level layer
     * @return true if the parsing has succeed else false
     */
    bool (*parse_data)(final struct Device* device, final Vehicle* vehicle);
    /**
     * Set a filter on the data received directly at the device level.
     * @return true if the filter setting success else false
     */
    bool (*set_filter_by_address)(final struct Device* device, ad_list_Buffer * filter_addresses);
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
    /**
     * Free this object
     */
    void (*free)(final struct Device* device);
} Device;

void device_location_set(final Device * device, final char *location);
const char * ad_device_type_as_string(AD_DEVICE_TYPE type);
AD_DEVICE_TYPE ad_device_type_from_str(char * type_str);

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
/**
 * If network location is an ip addresse + port
 */
bool device_location_is_network(final Device *device);
/**
 * Default handler for a device.
 */
const char * device_describe_state(final Device * device);

#endif
