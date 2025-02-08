#include "com/obd/saej1979/vehicle_info.h"

bool saej1979_vehicle_info_is_pid_supported(final OBDIFace* iface, final int pid) {
    return saej1979_is_pid_supported(iface, 9, 20, pid);
}


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
        ecu->name[0] = 0; \
        strncat(ecu->name, data->buffer, n); \
    }

SAEJ1979_VEHICLE_INFO_GENERATE_OBD_REQUEST_ITERATE_BODY(
    void*,saej1979_vehicle_info_discover_ecus_name,null,
    "0A",saej1979_vehicle_info_discover_ecus_names_iterator
)
