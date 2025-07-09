#ifndef __VEHICLE_INTERFACE_H
#define __VEHICLE_INTERFACE_H

#include "libautodiag/com/device.h"
#include "libautodiag/model/vehicle.h"

typedef struct {
    /**
     * Device onto which communication should start
     */
    Device* device;
    /**
     * Data structure representing the data received from the vehicle itself.
     */
    Vehicle * vehicle;
} VehicleIFace;

VehicleIFace* viface_new();
void viface_free(final VehicleIFace* iface);
void viface_lock(final VehicleIFace *iface);
void viface_unlock(final VehicleIFace *iface);
void viface_close(final VehicleIFace* iface);
/**
 * Send some data on the wire(less), without taking care of device implementation details
 * or communication protocol headers
 */
int viface_send(final VehicleIFace * iface, const char *command);
/**
 * Discover the type of device, ecus present on the bus and vehicle information.
 */
VehicleIFace* viface_open_from_device(final Device* device);

#endif