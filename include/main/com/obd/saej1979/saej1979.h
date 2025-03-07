/**
 * C API to interact with SAEJ1979 compliant vehicles.<br />
 * When there is multiple ECU on the line, the best possible is done regarding to the logic of the function.<br />
 * Example: for the number of dtc currently displayed we sum number of dtcs reported by each ecu.<br />
 */
#ifndef __OBD_SAEJ1979_H
#define __OBD_SAEJ1979_H

#include "com/obd/obd.h"

/**
 * Just iterate accross ecus received data until at least one valid found (validated by iterator setting the result value).
 * it works well for defining function of the shape eg saej1979_get_current_fuel_system_status(iface)
 * but passing parameters through the macro is a pain, for that writing custom data send function as
 * in saej1979_current_data_is_pid_supported is prefered.
 */
#define SAEJ1979_GENERATE_OBD_REQUEST_ITERATE(type,symbol,obd_request_str,iterator,errorValue,obd_data_buffer_accessor) type symbol(final OBDIFace* iface) { \
    SAEJ1979_GENERATE_OBD_REQUEST_ITERATE_BODY(type,obd_request_str,iterator,errorValue,obd_data_buffer_accessor) \
}

#define SAEJ1979_GENERATE_OBD_REQUEST_ITERATE_BODY(type,obd_request_str,iterator,errorValue,obd_data_buffer_accessor) \
    obd_lock(iface); \
    int response = 0; \
    type result = errorValue; \
    BUFFER obd_req = buffer_from_ascii_hex(obd_request_str); \
    int pid; \
    bool hasPid = ( 1 < obd_req->size ); \
    if ( hasPid ) { \
        pid = (int)((unsigned char)obd_req->buffer[1]); \
    } \
    obd_send(iface, obd_request_str); \
    obd_clear_data(iface); \
    response = obd_recv(iface); \
    if ( 0 < response ) { \
        if ( hasPid ) { \
            OBD_ITERATE_ECUS_DATA_BUFFER_WITH_PID(obd_data_buffer_accessor,iterator,pid); \
        } else { \
            OBD_ITERATE_ECUS_DATA_BUFFER(obd_data_buffer_accessor,iterator); \
        } \
    } \
    obd_unlock(iface); \
    return result; \

/**
 * Returned strings are malloc'ed
 */
char *saej1979_service_code_to_str(final unsigned char code);
bool saej1979_is_pid_supported(final OBDIFace* iface, final int service_id, final int pid_set_inc, int pid);

/**
 * Service 01 02
 */
#include "data.h"
/**
 * Service 03
 */
#include "stored_dtcs.h"
/**
 * Service 04
 */
bool saej1979_clear_dtc_and_stored_values(final OBDIFace* iface);
/**
 * Service 09
 */
#include "vehicle_info.h"

#endif
