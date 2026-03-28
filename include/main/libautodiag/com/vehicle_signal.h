#ifndef __AD_COM_VEHICLE_SIGNAL_H
#define __AD_COM_VEHICLE_SIGNAL_H

#include "libautodiag/lib.h"
#include "libautodiag/com/signal_input_builder.h"

/**
 * A signal over transport layer (ISO-TP for example).
 * The signal can be UDS, OBD, CAN
 * For now we simplify the signal result as a double
 * and the signal input is not parametrable, fixed in the stone
 */
AD_OBJECT_H(vehicle_signal,
    /**
     * 0
     */
    double rv_min;
    /**
     * 255
     */
    double rv_max;
    /**
     * $0
     */
    char * rv_formula;
    /**
     * $0${1}0D, service_id, framenumber?
     */
    char * input_formula;
    /**
     * Vehicle speed
     */
    char * name;
    /**
     * Speed of the vehicle
     */
    char * description;
    /**
     * ECM metrics
     */
    char * category;
    /**
     * eg. SAEJ1979
     */
    char * standard;
    /**
     * eg. VSS
     */
    char * slug;
    /**
     * eg. km/h
     */
    char * unit;
    /**
     * Source address of the signal (tester address)
     */
    Buffer * src_address;
    /**
     * Destination address of the signal (ecu)
     */
    Buffer * dst_address;
)

AD_HASHMAP_H(string, vehicle_signal)
#define AD_OBJECT_VEHICLE_SIGNAL_NO_STANDARD "UKN"
/**
 * eg. SAEJ1979.VSS
 */
char * ad_object_vehicle_signal_get_exec_path(ad_object_vehicle_signal *signal);
double ad_object_vehicle_signal_to_percent(ad_object_vehicle_signal *signal, double value);
/**
 * Retrieve a signal from the registry.
 */
ad_object_vehicle_signal * ad_signal_get(char * signal_location);
/**
 * Register a new signal in the registry.
 */
void ad_signal_put(ad_object_vehicle_signal * signal);

#define AD_SIGNAL_REGISTER( \
    param_name, \
    param_input_formula, \
    param_rv_min, \
    param_rv_max, \
    param_rv_formula, \
    param_description, \
    param_category, \
    param_standard, \
    param_slug, \
    param_unit, \
    param_src_address, \
    param_dst_address \
) { \
    ad_object_vehicle_signal *signal = ad_object_vehicle_signal_new(); \
    signal->rv_min = (param_rv_min); \
    signal->rv_max = (param_rv_max); \
    signal->rv_formula = ((param_rv_formula) != null ? strdup(param_rv_formula) : null); \
    signal->input_formula = ((param_input_formula) != null ? strdup(param_input_formula) : null); \
    signal->name = ((param_name) != null ? strdup(param_name) : null); \
    signal->description = ((param_description) != null ? strdup(param_description) : null); \
    signal->category = ((param_category) != null ? strdup(param_category) : null); \
    signal->standard = ((param_standard) != null ? strdup(param_standard) : strdup(AD_OBJECT_VEHICLE_SIGNAL_NO_STANDARD)); \
    signal->slug = ((param_slug) != null ? strdup(param_slug) : null); \
    signal->unit = ((param_unit) != null ? strdup(param_unit) : null); \
    signal->src_address = param_src_address == null ? null : ad_buffer_from_ascii_hex(param_src_address); \
    signal->dst_address = param_dst_address == null ? null : ad_buffer_from_ascii_hex(param_dst_address); \
    ad_signal_put(signal); \
}
void ad_signal_foreach(void (*cb)(ad_object_vehicle_signal *signal, void *userdata), void *userdata);
/**
 * Register hardcoded signals (eg. SAEJ1979)
 */
void ad_object_vehicle_signal_register_all();
ad_object_hashmap_string_vehicle_signal * ad_signals_get();

#endif