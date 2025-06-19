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
    Buffer_list * obd_data_buffer;
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
        Buffer_list * current_dtc;
        Buffer_list * pending_dtc;
        Buffer_list * permanent_dtc;
        /**
         * Data reported by the ECU
         */
        Buffer_list * current_data;
        Buffer_list * freeze_frame_data;
        Buffer_list * tests_results;        
        Buffer_list * tests_results_other;
        /**
         * Response trames to the clear request
         */
        Buffer_list * clear_dtc;
        Buffer_list * request_vehicle_information;
        Buffer_list * control_operation;
        Buffer_list * none;
    } obd_service;

} ECU;

ECU* vehicle_ecu_new();
void vehicle_ecu_free(ECU* ecu);
/**
 * Do not destruct the address and so on, only empty.
 */
void vehicle_ecu_empty(ECU* ecu);
#define VEHICLE_YEAR_EMPTY -1

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
    Buffer_list * obd_data_buffer;
    Buffer * vin;
    char *country;
    /**
     * eg Citroen
     */
    char *manufacturer;
    /**
     * Make year of the car
     */
    int year;
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

LIST_DEFINE_WITH_MEMBERS_AUTO(Vehicle)
void Vehicle_list_empty(Vehicle_list * list);

#endif
