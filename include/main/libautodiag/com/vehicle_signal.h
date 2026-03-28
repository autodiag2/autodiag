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
    double rv_min;
    double rv_max;
    char * rv_formula;
    char * input_formula;
    char * name;
    char * description;
    char * category;
    char * standard;
    char * unit;
)

double ad_object_vehicle_signal_to_percent(ad_object_vehicle_signal *signal, double value);

#endif