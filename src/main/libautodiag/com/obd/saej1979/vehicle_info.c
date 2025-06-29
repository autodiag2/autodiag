#include "libautodiag/com/obd/saej1979/vehicle_info.h"

bool saej1979_vehicle_info_is_pid_supported(final OBDIFace* iface, final int pid) {
    return saej1979_is_pid_supported(iface, 9, 20, pid);
}

#define saej1979_vehicle_info_discover_vin_iterator(data) \
    if ( 17 == data->size ) { \
        iface->vehicle->vin = buffer_copy(data); \
    } else { \
        log_msg(LOG_ERROR, "Incorrect VIN: size=%d content=%s", data->size, buffer_to_ascii(data)); \
    }

SAEJ1979_VEHICLE_INFO_GENERATE_OBD_REQUEST_ITERATE_BODY(
    void*,saej1979_vehicle_info_discover_vin,null,
    "02",saej1979_vehicle_info_discover_vin_iterator
)

#define saej1979_vehicle_info_discover_ecus_names_iterator(data) \
    if ( 0 < data->size ) { \
        final int n; \
        if ( data->buffer[data->size-1] == 0x00 ) { \
            n = strlen(data->buffer); \
        } else { \
            n = data->size; \
        } \
        if ( ecu->name != null ) { \
            free(ecu->name); \
        } \
        ecu->name = (char*)malloc(sizeof(char) * (n + 1)); \
        ecu->name[n] = 0; \
        strncat(ecu->name, data->buffer, n); \
    }

SAEJ1979_VEHICLE_INFO_GENERATE_OBD_REQUEST_ITERATE_BODY(
    void*,saej1979_vehicle_info_discover_ecus_name,null,
    "0A",saej1979_vehicle_info_discover_ecus_names_iterator
)
