#include "libautodiag/com/vehicle_signal.h"

AD_OBJECT_SRC(vehicle_signal)

double ad_object_vehicle_signal_to_percent(ad_object_vehicle_signal *signal, double value) {
    if (signal == null) {
        return 0.0;
    }

    if (signal->rv_max == signal->rv_min) {
        return 0.0;
    }

    return ((value - signal->rv_min) / (signal->rv_max - signal->rv_min)) * 100.0;
}
ad_object_vehicle_signal * ad_object_vehicle_signal_new() {
    ad_object_vehicle_signal * signal = (ad_object_vehicle_signal*)malloc(sizeof(ad_object_vehicle_signal));
    signal->rv_min = 0;
    signal->rv_max = 0;
    signal->rv_formula = null;
    signal->input_formula = null;
    signal->name = null;
    signal->description = null;
    signal->category = null;
    signal->standard = null;
    signal->unit = null;
    return signal;
}
void ad_object_vehicle_signal_free(ad_object_vehicle_signal* signal) {
    if ( signal != null ) {
        MEMORY_FREE_POINTER(signal->rv_formula);
        MEMORY_FREE_POINTER(signal->input_formula);
        MEMORY_FREE_POINTER(signal->name);
        MEMORY_FREE_POINTER(signal->description);
        MEMORY_FREE_POINTER(signal->category);
        MEMORY_FREE_POINTER(signal->standard);
        MEMORY_FREE_POINTER(signal->unit);
        MEMORY_FREE_POINTER(signal);
    }
}
ad_object_vehicle_signal* ad_object_vehicle_signal_assign(ad_object_vehicle_signal* to, ad_object_vehicle_signal* from) {
    to->rv_min = from->rv_min;
    to->rv_max = from->rv_max;
    to->rv_formula = strdup(from->rv_formula);
    to->input_formula = strdup(from->input_formula);
    to->name = strdup(from->name);
    to->description = strdup(from->description);
    to->category = strdup(from->category);
    to->standard = strdup(from->standard);
    to->unit = strdup(from->unit);
    return to;
}