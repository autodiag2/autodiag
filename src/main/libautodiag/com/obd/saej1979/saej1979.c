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

bool saej1979_clear_dtc_and_stored_values(final OBDIFace* iface) {
    obd_lock(iface);
    obd_send(iface, "04");
    obd_clear_data(iface);
    obd_recv(iface);
    obd_unlock(iface);
    return true;
}

#define saej1979_is_pid_supported_iterator(data) \
    int buffer_as_32bits = 0; \
    final unsigned char * ptr = (unsigned char*)&buffer_as_32bits; \
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

bool saej1979_is_pid_supported(final OBDIFace* iface, final int service_id, final int pid_set_inc, int pid) {
    assert(0 <= pid);
    if ( 0 == pid ) {
        return true;
    } else {
        pid -= 1;
        final int current_set = pid - (pid % pid_set_inc);
        final int pid_as_bitmask = 1 << (pid - current_set);

        char * request;
        asprintf(&request,"%02x%02x", service_id, current_set);

        obd_lock(iface);
        bool result = false;

        obd_send(iface, request);
        obd_clear_data(iface);
        obd_recv(iface);
        if ( service_id == 0x01 ) {
            OBD_ITERATE_ECUS_DATA_BUFFER_WITH_PID(ecu->obd_service.current_data,saej1979_is_pid_supported_iterator,current_set)
        } else if ( service_id == 0x02 ) {
            OBD_ITERATE_ECUS_DATA_BUFFER_WITH_PID(ecu->obd_service.freeze_frame_data,saej1979_is_pid_supported_iterator,current_set)
        } else if ( service_id == 0x09 ) {
            OBD_ITERATE_ECUS_DATA_BUFFER_WITH_PID(ecu->obd_service.request_vehicle_information,saej1979_is_pid_supported_iterator,current_set)
        }
        obd_unlock(iface);
    

        free(request);
        return result;
    }
}
