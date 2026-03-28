#include "libautodiag/com/vehicle_signal.h"
#include "libautodiag/com/obd/saej1979/data.h"

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
void ad_object_vehicle_signal_register_all() {
    ad_saej1979_data_register_signals();
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
    signal->slug = null;
    signal->dst_address = null;
    signal->src_address = null;
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
        MEMORY_FREE_POINTER(signal->slug);
        ad_buffer_free(signal->dst_address);
        ad_buffer_free(signal->src_address);
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
    to->slug = strdup(from->slug);
    to->dst_address = ad_buffer_copy(from->dst_address);
    to->src_address = ad_buffer_copy(from->src_address);
    return to;
}
char * ad_object_vehicle_signal_get_exec_path(ad_object_vehicle_signal *signal) {
    assert(signal != null);
    return gprintf("%s.%s",
        signal->standard == null ? AD_OBJECT_VEHICLE_SIGNAL_NO_STANDARD : signal->standard,
        signal->slug
    );
}
static ad_object_hashmap_string_vehicle_signal * ad_signals_registry = null;
ad_object_hashmap_string_vehicle_signal * ad_signals_get() {
    if ( ad_signals_registry == null ) {
        ad_signals_registry = ad_object_hashmap_string_vehicle_signal_new();
    }
    return ad_signals_registry;
}
ad_object_vehicle_signal * ad_signal_get(char * signal_location) {
    ad_object_hashmap_string_vehicle_signal * registry = ad_signals_get();
    ad_object_string * key = ad_object_string_new_from(signal_location);
    ad_object_vehicle_signal * signal = 
        ad_object_hashmap_string_vehicle_signal_get(
            registry,
            key
        );
    ad_object_string_free(key);
    return signal;
}
void ad_signal_put(ad_object_vehicle_signal * signal) {
    ad_object_hashmap_string_vehicle_signal * registry = ad_signals_get();
    ad_object_string * key = ad_object_string_new_from(ad_object_vehicle_signal_get_exec_path(signal));
    ad_object_hashmap_string_vehicle_signal_set(registry, key, signal);
}
int ad_object_hashmap_string_vehicle_signal_key_comparator(ad_object_string * k1, ad_object_string * k2) {
    if ( (k1->data == null) ^ (k2->data == null) ) {
        return 1;
    }
    return strcmp(k1->data, k2->data);
}
ad_object_hashmap_string_vehicle_signal * ad_object_hashmap_string_vehicle_signal_assign(ad_object_hashmap_string_vehicle_signal* to, ad_object_hashmap_string_vehicle_signal*from) {
    log_err("TODO");
    return to;
}
AD_HASHMAP_SRC(string, vehicle_signal)
typedef struct {
    void (*cb)(ad_object_vehicle_signal *signal, void *userdata);
    void *userdata;
} ad_signal_foreach_ctx;

static void ad_signal_foreach_iter(char *key, ad_object_vehicle_signal *signal, void *userdata) {
    ad_signal_foreach_ctx *ctx = (ad_signal_foreach_ctx*)userdata;
    if (signal != null && ctx->cb != null) {
        ctx->cb(signal, ctx->userdata);
    }
}

void ad_signal_foreach(void (*cb)(ad_object_vehicle_signal *signal, void *userdata), void *userdata) {
    unsigned i;

    if (cb == null) {
        return;
    }
    if (ad_signals_registry == null) {
        return;
    }

    for (i = 0; i < ad_signals_registry->size; i++) {
        ad_object_vehicle_signal *signal = ad_signals_registry->values[i];
        if (signal != null) {
            cb(signal, userdata);
        }
    }
}