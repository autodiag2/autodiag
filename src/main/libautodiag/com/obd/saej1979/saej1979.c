#include "libautodiag/com/obd/saej1979/saej1979.h"

char *saej1979_service_code_to_str(final unsigned char code) {
    switch(code) {
        case 0x01: return strdup("Show current data");
        case 0x02: return strdup("Show freeze frame data");
        case 0x03: return strdup("Show stored Diagnostic Trouble Codes");
        case 0x04: return strdup("Clear Diagnostic Trouble Codes and stored values");
        //case 0x05: return strdup("Test results, oxygen sensor monitoring"); // (non can only)
        //case 0x06: return strdup("Test results, other components/system monitoring (Test results, oxygen sensor monitoring for CAN only)");
        case 0x07: return strdup("Show pending Diagnostic Trouble Codes (detected during current or last driving cycle)");
        case 0x08: return strdup("Control operation of on-board component/system");
        case 0x09: return strdup("Request vehicle information");
        case 0x0A: return strdup("Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)");
        default:
            return null;
    }
}

bool saej1979_clear_dtc_and_stored_values(final VehicleIFace* iface) {
    viface_lock(iface);
    viface_send(iface, "04");
    viface_clear_data(iface);
    viface_recv(iface);
    viface_unlock(iface);
    return true;
}

#define saej1979_is_pid_supported_iterator(data) \
    int buffer_as_32bits = 0; \
    final byte * ptr = (byte*)&buffer_as_32bits; \
    if ( O32_HOST_ORDER == O32_LITTLE_ENDIAN ) { \
        memcpy(&buffer_as_32bits,data->buffer, 4); \
    } else if ( O32_HOST_ORDER == O32_BIG_ENDIAN ) { \
        for(int i = 0; i < 4; i++) { \
            *(ptr+i) = data->buffer[3 - i]; \
        } \
    } else { \
        log_msg(LOG_ERROR, "unsupported host orderering"); \
    } \
    for(int i = 0; i < 4; i ++) { \
        ptr[i] = byte_reverse(ptr[i]); \
    } \
    result |= (pid_as_bitmask&buffer_as_32bits) != 0;

int int_cmp(int* e1, int* e2) {
    return *e1 - *e2;
}
AD_LIST_SRC(int)

list_int * saej1979_is_pids_supported(final VehicleIFace* iface, final int service_id, int pid) {
    assert(0 <= pid);
    final int pid_set_inc;
    if ( service_id == 0x01 ) {
        pid_set_inc = 0x20;
    } else if ( service_id == 0x02 ) {
        pid_set_inc = 0x20;
    } else if ( service_id == 0x09 ) {
        pid_set_inc = 0x20;
    } else {
        log_msg(LOG_ERROR, "called with the wrong service id : %d", service_id);
        exit(1);
    }
    list_int * pids_status = list_int_new();
    if ( 0 == pid ) {
        list_int_append(pids_status, intdup(true));
    } else {
        pid -= 1;
        final int current_set = pid - (pid % pid_set_inc);
        final int pid_as_bitmask = 1 << (pid - current_set);

        char * request;
        asprintf(&request,"%02x%02x", service_id, current_set);

        viface_lock(iface);
        bool result = false;

        viface_send(iface, request);
        viface_clear_data(iface);
        viface_recv(iface);
        if ( service_id == 0x01 ) {
            OBD_ITERATE_ECUS_DATA_BUFFER_WITH_PID(ecu->obd_service.current_data,saej1979_is_pid_supported_iterator,current_set)
        } else if ( service_id == 0x02 ) {
            OBD_ITERATE_ECUS_DATA_BUFFER_WITH_PID(ecu->obd_service.freeze_frame_data,saej1979_is_pid_supported_iterator,current_set)
        } else if ( service_id == 0x09 ) {
            OBD_ITERATE_ECUS_DATA_BUFFER_WITH_PID(ecu->obd_service.request_vehicle_information,saej1979_is_pid_supported_iterator,current_set)
        }
        viface_unlock(iface);
        list_int_append(pids_status, intdup(result));

        free(request);
    }
    return pids_status;
}
bool saej1979_is_pid_supported(final VehicleIFace* iface, final int service_id, int pid) {
    list_int * pids_status = saej1979_is_pids_supported(iface, service_id, pid);
    final bool result = *pids_status->list[0];
    list_int_free(pids_status);
    return result;
}