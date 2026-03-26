#include "libautodiag/com/obd/saej1979/vehicle_info.h"

bool saej1979_vehicle_info_is_pid_supported(final VehicleIFace* iface, final int pid) {
    return saej1979_is_pid_supported(iface, 9, pid);
}

#define saej1979_vehicle_info_discover_vin_iterator(data) \
    if ( 0 < data->size ) { \
        Device * device = iface->device; \
        if ( device->type == AD_DEVICE_TYPE_SERIAL ) { \
            ELMDevice * elm = (ELMDevice*)device; \
            if ( elm->proto_is_can(AD_DEVICE(elm)) ) { \
                byte data_items = data->buffer[0]; \
                if ( 1 < data_items ) { \
                    log_err("should reply only a single data item"); \
                } else if ( data_items == 0 ) { \
                    log_warn("no data item, can find the VIN"); \
                } else { \
                    ad_buffer_recycle(iface->vehicle->vin); \
                    ad_buffer_slice_append(iface->vehicle->vin, data, 1, data->size-1); \
                } \
            } else { \
                ad_buffer_recycle(iface->vehicle->vin); \
                int messageCount = 0; \
                for(int data_i = 0; data_i < data->size; data_i++) { \
                    if ( (data_i % 5) == 0 ) { \
                        messageCount ++; \
                        if ( messageCount != data->buffer[data_i] ) { \
                            log_err("missordering detecting in received infotype vin should reorder"); \
                        } \
                        continue; \
                    } \
                    ad_buffer_append_byte(iface->vehicle->vin, data->buffer[data_i]); \
                } \
            } \
        } else { \
            iface->vehicle->vin = ad_buffer_copy(data); \
        } \
    }

SAEJ1979_VEHICLE_INFO_GENERATE_OBD_REQUEST_ITERATE_BODY(
    void*,saej1979_vehicle_info_discover_vin,null,
    "02",saej1979_vehicle_info_discover_vin_iterator
)

#define saej1979_vehicle_info_discover_ecus_names_iterator(data) \
    if (0 < data->size) { \
        int n; \
        if (data->buffer[data->size - 1] == 0x00) { \
            n = strlen((char*)data->buffer); \
        } else { \
            n = data->size; \
        } \
        MEMORY_FREE_POINTER(ecu->name); \
        ecu->name = malloc(n + 1); \
        memcpy(ecu->name, data->buffer, n); \
        ecu->name[n] = '\0'; \
        vehicle_event_emit_on_ecu_register(iface->vehicle, ecu); \
    }

SAEJ1979_VEHICLE_INFO_GENERATE_OBD_REQUEST_ITERATE_BODY(
    void*,saej1979_vehicle_info_discover_ecus_name,null,
    "0A",saej1979_vehicle_info_discover_ecus_names_iterator
)
