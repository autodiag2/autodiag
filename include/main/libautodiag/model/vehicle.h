/**
 * Dynamic information retrieved from the car (eg not loaded from database files)
 */
#ifndef __VEHICLE_H
#define __VEHICLE_H

#include "libautodiag/lib.h"
#include "libautodiag/buffer.h"

typedef struct {
    /**
     * Data buffer that has been sent by this ECU.
     */
    BufferList * obd_data_buffer;
    /**
     * The source address of the message.
     */
    Buffer * address;
    /**
     * Name of this ECU.
     */
    char *name;
   /**
     * eg BOSCH MED 17.4
     */
    char *model;
    /**
     * Each ECU provide an obd service, each data recevied for
     * those services is divided and putted there
     */
    struct {
        /**
         * DTC reported by this ECU
         */
        BufferList * current_dtc;
        BufferList * pending_dtc;
        BufferList * permanent_dtc;
        /**
         * Data reported by the ECU
         */
        BufferList * current_data;
        BufferList * freeze_frame_data;
        BufferList * tests_results;        
        BufferList * tests_results_other;
        /**
         * Response trames to the clear request
         */
        BufferList * clear_dtc;
        BufferList * request_vehicle_information;
        BufferList * control_operation;
        BufferList * none;
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
    BufferList * obd_data_buffer;
    /**
     * eg Citroen
     */
    char *brand;
    /**
     * eg EP6DT
     */
    char *engine;
    struct {
        /**
         * Directory containing meta datas for this car
         */
        char *directory;
    } internal;
} Vehicle;

Vehicle* vehicle_new();
void vehicle_free(Vehicle* v);
void vehicle_fill_global_obd_data_from_ecus(Vehicle* v);
void vehicle_dump(Vehicle* v);
void vehicle_debug(Vehicle* v);

ECU* vehicle_ecu_add(Vehicle* v, byte* address, int size);
void vehicle_ecu_debug(final ECU *ecu);
ECU* vehicle_ecu_add_if_not_in(Vehicle* v, byte* address, int size);
void vehicle_ecu_empty_duplicated_info(ECU* ecu);

LIST_DEFINE_WITH_MEMBERS(VehicleList,Vehicle)
void VehicleList_empty(VehicleList * list);

#endif
