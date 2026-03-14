/**
 * Dynamic information retrieved from the car (eg not loaded from database files)
 */
#ifndef __AD_VEHICLE_H
#define __AD_VEHICLE_H

#include "libautodiag/lib.h"
#include "libautodiag/buffer.h"

AD_OBJECT_H(ECU,
    /**
     * Binary data (as hex string when on the interecting 
     * through serial devices) received from this ECU.
     */
    ad_list_Buffer * data_buffer;
    /**
     * The source address of the message.
     */
    Buffer * address;
    /**
     * Name of this ECU.
     */
    char *name;
    /**
     * BOSCH
     */
    char * manufacturer;
    /**
     * eg MED 17.4
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
        ad_list_Buffer * current_dtc;
        ad_list_Buffer * pending_dtc;
        ad_list_Buffer * permanent_dtc;
        /**
         * Data reported by the ECU
         */
        ad_list_Buffer * current_data;
        ad_list_Buffer * freeze_frame_data;
        ad_list_Buffer * tests_results;        
        ad_list_Buffer * tests_results_other;
        /**
         * Response trames to the clear request
         */
        ad_list_Buffer * clear_dtc;
        ad_list_Buffer * request_vehicle_information;
        ad_list_Buffer * control_operation;
        ad_list_Buffer * none;
    } obd_service;

);

/**
 * Do not destruct the address and so on, only empty.
 */
void vehicle_ecu_empty(ad_object_ECU* ecu);
#define VEHICLE_YEAR_EMPTY -1

AD_LIST_H(ad_object_ECU)

typedef struct {
    /**
     * List of ECUs currently detected in the vehicle.
     */
    ad_list_ad_object_ECU * ecus;
    /**
     * Binary data (as hex string when on the interecting 
     * through serial devices) received from this vehicle (any ECU).
     */
    ad_list_Buffer * data_buffer;
    Buffer * vin;
    char *country;
    /**
     * eg Citroen
     */
    char *manufacturer;
    /**
     * C5 X7
     */
    char * model;
    /**
     * Make year of the car
     */
    int year;
    /**
     * eg EP6DT
     */
    char *engine;
    /**
     * eg Citroen
     */
    char * engine_manufacturer;
    struct {
        struct {
            EventHandlerHolder * onECURegister;
            EventHandlerHolder * onFilterChange;
        } events;
        ad_list_Buffer * filter;
    } internal;
} Vehicle;

#define vehicle_event_emit_on_ecu_register(v, ecu) \
    ehh_trigger(v->internal.events.onECURegister, (void(*)(ad_object_ECU*)), ecu);

#define vehicle_event_emit_on_filter_change(v, type, address) \
    ehh_trigger(v->internal.events.onFilterChange, (void(*)(char*,Buffer*)), type, address)
#define vehicle_event_emit_on_filter_change_add(v, address) \
    vehicle_event_emit_on_filter_change(v, "add", address)
#define vehicle_event_emit_on_filter_change_rm(v, address) \
    vehicle_event_emit_on_filter_change(v, "rm", address)
#define vehicle_event_emit_on_filter_change_clear(v) \
    vehicle_event_emit_on_filter_change(v, "clear", null)

Vehicle* vehicle_new();
void vehicle_free(Vehicle* v);
void vehicle_fill_global_data_buffer_from_ecus(Vehicle* v);
void vehicle_dump(Vehicle* v);
void vehicle_debug(Vehicle* v);

ad_object_ECU* vehicle_ecu_add(Vehicle* v, byte* address, int size);
void vehicle_ecu_debug(final ad_object_ECU *ecu);
ad_object_ECU* vehicle_ecu_add_if_not_in(Vehicle* v, byte* address, int size);
void vehicle_ecu_empty_duplicated_info(ad_object_ECU* ecu);
ad_object_ECU* vehicle_search_ecu_by_address(Vehicle* v, Buffer* address);

AD_LIST_H(Vehicle)
void ad_list_Vehicle_empty(ad_list_Vehicle * list);

#endif
