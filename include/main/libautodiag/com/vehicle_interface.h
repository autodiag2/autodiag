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

#endif