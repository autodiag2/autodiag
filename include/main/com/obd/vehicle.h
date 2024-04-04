/**
 * Dynamic information retrieved from the car (eg not loaded from database files)
 */
#ifndef __VEHICLE_H
#define __VEHICLE_H

#include "lib/lib.h"
#include "lib/buffer.h"

typedef struct {
    /**
     * Data buffer that has been sent by this ECU.
     */
    BUFFERLIST obd_data_buffer;
    /**
     * The source address of the message.
     */
    BUFFER address;
    /**
     * Name of this ECU.
     */
    char *name;

    /**
     * Each ECU provide an obd service, each data recevied for
     * those services is divided and putted there
     */
    struct {
        /**
         * DTC reported by this ECU
         */
        BUFFERLIST current_dtc;
        BUFFERLIST pending_dtc;
        BUFFERLIST permanent_dtc;
        /**
         * Data reported by the ECU
         */
        BUFFERLIST current_data;
        BUFFERLIST freeze_frame_data;
        BUFFERLIST tests_results;        
        BUFFERLIST tests_results_other;
        /**
         * Response trames to the clear request
         */
        BUFFERLIST clear_dtc;
        BUFFERLIST request_vehicle_information;
        BUFFERLIST control_operation;
        BUFFERLIST none;
    } obd_service;

} ECU;

ECU* vehicle_ecu_new();
void vehicle_ecu_free(ECU* ecu);
/**
 * Do not destruct the address and so on, only empty.
 */
void vehicle_ecu_empty(ECU* ecu);

typedef struct {
    /**
     * List of ECUs currently detected in the vehicle.
     */
    ECU ** ecus;
    int ecus_len;
    /**
     * Contains the OBD data without taking care of which ecu has responded.
     * When we do not use headers or when we use headers but want to use in a more concise way.
     */
    BUFFERLIST obd_data_buffer;
} Vehicle;

Vehicle* vehicle_new();
void vehicle_free(Vehicle* v);
ECU* vehicle_ecu_add(Vehicle* v, byte* address, int size);
ECU* vehicle_ecu_add_if_not_in(Vehicle* v, byte* address, int size);
void vehicle_fill_global_obd_data_from_ecus(Vehicle* v);
void vehicle_dump(Vehicle* v);

#define VEHICLE Vehicle*

#endif
