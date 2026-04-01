#include "libautodiag/com/obd/saej1979/vehicle_info.h"

bool saej1979_vehicle_info_is_pid_supported(final VehicleIFace* iface, final int pid) {
    return saej1979_is_pid_supported(iface, OBD_SERVICE_REQUEST_VEHICLE_INFORMATION, pid);
}
static Buffer * saej1979_info_type_retrieve(Device * device, Buffer * data) {
    if ( 0 == data->size ) {
        return ad_buffer_new(); 
    }
    Buffer * info_type_result = ad_buffer_new();
    if ( device->type == AD_DEVICE_TYPE_SERIAL ) {
        ELMDevice * elm = (ELMDevice*)device;
        if ( elm->proto_is_can(AD_DEVICE(elm)) ) {
            byte data_items = data->buffer[0];
            if ( 1 < data_items ) {
                log_err("should reply only a single data item");
            } else if ( data_items == 0 ) {
                log_warn("no data item, can find the VIN");
            } else {
                ad_buffer_slice_append(info_type_result, data, 1, data->size-1);
            }
        } else {
            log_err("parsing of non CAN vehicle info should take care of header and other things");
            int messageCount = 0;
            for(int data_i = 0; data_i < data->size; data_i++) {
                if ( (data_i % 5) == 0 ) {
                    messageCount ++;
                    if ( messageCount != data->buffer[data_i] ) {
                        log_err("missordering detecting in received infotype vin should reorder"); \
                    }
                    continue;
                }
                ad_buffer_append_byte(info_type_result, data->buffer[data_i]);
            }
        }
    } else {
        info_type_result = ad_buffer_copy(data);
    }
    return info_type_result;
}
#define saej1979_vehicle_info_discover_vin_iterator(data) \
    iface->vehicle->vin = saej1979_info_type_retrieve(iface->device, data); \

SAEJ1979_VEHICLE_INFO_GENERATE_OBD_REQUEST_ITERATE_BODY(
    void*,saej1979_vehicle_info_discover_vin,null,
    "02",saej1979_vehicle_info_discover_vin_iterator
)

#define saej1979_vehicle_info_discover_ecus_names_iterator(data) \
    if (0 < data->size) { \
        Buffer * ecu_name_bin = saej1979_info_type_retrieve(iface->device, data); \
        if ( 0 < ecu_name_bin->size ) { \
            int n; \
            if (ecu_name_bin->buffer[ecu_name_bin->size - 1] == 0x00) { \
                n = strlen((char*)ecu_name_bin->buffer); \
            } else { \
                n = ecu_name_bin->size; \
            } \
            MEMORY_FREE_POINTER(ecu->name); \
            ecu->name = malloc(n + 1); \
            memcpy(ecu->name, ecu_name_bin->buffer, n); \
            ecu->name[n] = '\0'; \
            vehicle_event_emit_on_ecu_register(iface->vehicle, ecu); \
        } \
    }

SAEJ1979_VEHICLE_INFO_GENERATE_OBD_REQUEST_ITERATE_BODY(
    void*,saej1979_vehicle_info_discover_ecus_name,null,
    "0A",saej1979_vehicle_info_discover_ecus_names_iterator
)
