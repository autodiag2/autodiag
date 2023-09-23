/**
 * Dynamic information retrieved from the car (eg not loaded from database files)
 */
#ifndef __VEHICLE_H
#define __VEHICLE_H

typedef struct {
    /**
     * Hexadecimal address of the onboard computer in ascii hexa.
     */
    char *address;
} ECU;

typedef struct {
    /**
     * List of ECUs currently detected in the vehicle.
     */
    ECU * list;
} Vehicle;

#endif
