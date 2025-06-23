/**
 * Include this header will include all obd devices and any specification related to OBD
 */
#ifndef __OBD_OBD_H
#define __OBD_OBD_H

#include "libautodiag/com/device.h"
#include "libautodiag/com/vehicle_interface.h"
#include "libautodiag/model/iso3779.h"

/**
 * Also referred as modes previously
 */
typedef enum {
    OBD_SERVICE_NONE,OBD_SERVICE_SHOW_CURRENT_DATA,
    OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA,OBD_SERVICE_SHOW_DTC,
    OBD_SERVICE_CLEAR_DTC,OBD_SERVICE_TESTS_RESULTS,
    OBD_SERVICE_TESTS_RESULTS_OTHER,
    OBD_SERVICE_PENDING_DTC,OBD_SERVICE_CONTROL_OPERATION,
    OBD_SERVICE_REQUEST_VEHICLE_INFORMATION,
    OBD_SERVICE_PERMANENT_DTC
} OBDServiceId;

#define OBD_DIAGNOSTIC_SERVICE_NEGATIVE_RESPONSE 0x7F
#define OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE 0x40

typedef struct {
    VehicleIFace;
} OBDIFace;

OBDIFace* obd_new();
OBDIFace* obd_new_from_device(final Device* device);
void obd_free(final OBDIFace* iface);
void obd_lock(final OBDIFace *iface);
void obd_unlock(final OBDIFace *iface);
void obd_fill_infos_from_vin(final OBDIFace * iface);

/**
 * Send an OBD query
 */
int obd_send(final OBDIFace * iface, const char *command);

/**
 * From obd point of view we do not care what ECU has sent the message we only focus on the message "the car has sent back theses errors".
 * @return number of data buffer received or OBD_RECV_ERROR on error (no obd data available, serial error or any error more deep in the stack)
 */
int obd_recv(final OBDIFace* iface);
/**
 * Generic error (eg any device reception error)
 */
#define OBD_RECV_ERROR -1
/**
 * The vehicle does not support the wanted function
 */
#define OBD_RECV_NO_DATA -2

void obd_clear_data(final OBDIFace* iface);
void obd_close(final OBDIFace* iface);
/**
 * Parse one obd message (non can devices only)
 */
bool obd_standard_parse_buffer(final Vehicle* vehicle, final Buffer* bin_buffer);

/**
 * handler has the shape: handler(Buffer * data, arg)
 */
#define OBD_ITERATE_ECUS_DATA_BUFFER(list_Buffer,handler) { \
    final Vehicle* v = iface->vehicle; \
    for(int i = 0; i < v->ecus_len; i++) { \
        final ECU* ecu = v->ecus[i]; \
        for(int j = 0; j < list_Buffer->size; j++) { \
            Buffer * data = list_Buffer->list[j]; \
            handler(data); \
        } \
    } \
}

/**
 * handler has the shape: handler(Buffer * data, arg) 
 * and is called only when the pid match the wanted one
 * ex: 
 *  OBD_ITERATE_ECUS_DATA_BUFFER_WITH_PID(ecu->obd_service.current_data, saej1979_get_current_vehicle_speed_iterator, 0x0D, result);
 */
#define OBD_ITERATE_ECUS_DATA_BUFFER_WITH_PID(list_Buffer,handler,_expected_pid) { \
    int expected_pid = _expected_pid; \
    final Vehicle* v = iface->vehicle; \
    for(int i = 0; i < v->ecus_len; i++) { \
        final ECU* ecu = v->ecus[i]; \
        for(int j = 0; j < list_Buffer->size; j++) { \
            Buffer * data = list_Buffer->list[j]; \
            if ( 0 < data->size ) { \
                Buffer * data_copy = buffer_copy(data); \
                byte pid = buffer_extract_0(data_copy); \
                if ( expected_pid == pid ) { \
                    handler(data_copy); \
                } \
            } \
        } \
    } \
}

/**
 * A one time process to discover ECUs on the bus, and specific vehicle data
 */
void obd_discover_vehicle(OBDIFace* iface);
/**
 * Discover the type of device, ecus present on the bus and vehicle information.
 */
OBDIFace* obd_open_from_device(final Device* device);

#include "iso15031/iso15031.h"
#include "libautodiag/com/can/can.h"
#include "libautodiag/com/serial/elm/elm.h"

#endif
