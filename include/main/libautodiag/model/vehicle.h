/**
 * Dynamic information retrieved from the car (eg not loaded from database files)
 */
#ifndef __VEHICLE_H
#define __VEHICLE_H

#include "libautodiag/lib.h"
#include "libautodiag/buffer.h"

typedef struct {
    /**
     * Binary data (as hex string when on the interecting 
     * through serial devices) received from this ECU.
     */
    list_Buffer * data_buffer;
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
        list_Buffer * current_dtc;
        list_Buffer * pending_dtc;
        list_Buffer * permanent_dtc;
        /**
         * Data reported by the ECU
         */
        list_Buffer * current_data;
        list_Buffer * freeze_frame_data;
        list_Buffer * tests_results;        
        list_Buffer * tests_results_other;
        /**
         * Response trames to the clear request
         */
        list_Buffer * clear_dtc;
        list_Buffer * request_vehicle_information;
        list_Buffer * control_operation;
        list_Buffer * none;
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
     * Binary data (as hex string when on the interecting 
     * through serial devices) received from this vehicle (any ECU).
     */
    list_Buffer * data_buffer;
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
        struct {
            EventHandlerHolder * onECURegister;
        } events;
    } internal;
} Vehicle;

#define VEHICLE_EVENTS_ON_ECU_REGISTER_CAST (void(*)(ECU*))
#define vehicle_events_on_ecu_register(v, ecu) \
    ehh_trigger(v->internal.events.onECURegister, VEHICLE_EVENTS_ON_ECU_REGISTER_CAST, ecu);

Vehicle* vehicle_new();
void vehicle_free(Vehicle* v);
void vehicle_fill_global_data_buffer_from_ecus(Vehicle* v);
void vehicle_dump(Vehicle* v);
void vehicle_debug(Vehicle* v);

ECU* vehicle_ecu_add(Vehicle* v, byte* address, int size);
void vehicle_ecu_debug(final ECU *ecu);
ECU* vehicle_ecu_add_if_not_in(Vehicle* v, byte* address, int size);
void vehicle_ecu_empty_duplicated_info(ECU* ecu);

AD_LIST_H(Vehicle)
void list_Vehicle_empty(list_Vehicle * list);

#endif
