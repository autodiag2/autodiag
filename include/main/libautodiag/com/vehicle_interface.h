#ifndef __VEHICLE_INTERFACE_H
#define __VEHICLE_INTERFACE_H

#include "libautodiag/com/device.h"
#include "libautodiag/model/vehicle.h"

typedef enum {
    VIFaceState_READY,
    VIFaceState_NOT_READY
}VIFaceState;
typedef struct {
    /**
     * Device onto which communication should start.
     */
    Device* device;
    /**
     * Data structure representing the data received from the vehicle itself.
     */
    Vehicle * vehicle;
    /**
     * State of this interface, whether ready to communicate with the vehicle or not.
     */
    int state;
    /**
     * Does have uds on the current connexion.
     */
    bool uds_enabled;
    /**
     * A thread that send periodically TesterPresent messages.
     */
    pthread_t *uds_tester_present_timer;
    struct {
        EventHandlerHolder * onRequest;
        EventHandlerHolder * onResponse;
    } internal;
} VehicleIFace;

#define viface_event_emit_on_request(iface, binRequest) \
    ehh_trigger(iface->internal.onRequest, (void(*)(Buffer*)), binRequest);

#define viface_event_emit_on_response(iface, ecu, binResponse) \
    ehh_trigger(iface->internal.onRequest, (void(*)(ECU*,Buffer*)), ecu, binResponse);

VehicleIFace* viface_new();
void viface_free(final VehicleIFace* iface);
void viface_lock(final VehicleIFace *iface);
void viface_unlock(final VehicleIFace *iface);
void viface_close(final VehicleIFace* iface);
/**
 * Send some data on the wire(less), without taking care of device implementation details
 * or communication protocol headers
 */
int viface_send(final VehicleIFace * iface, const char *command);
/**
 * Generic error (eg any device reception error)
 */
#define VIFACE_RECV_ERROR -1
/**
 * The vehicle does not support the wanted function
 */
#define VIFACE_RECV_UNSUPPORTED -2
/**
 * Open or reopen a device and configure it.
 * Discover the type of device, ecus present on the bus and vehicle information.
 */
VehicleIFace* viface_open_from_device(final Device* device);
/**
 * Same but provide the VehicleIFace object
 */
bool viface_open_from_iface_device(final VehicleIFace * iface, final Device* device);
/**
 * Receive some data on the device and update the model
 * @return number of data buffer received or VIFACE_RECV_ERROR on error (no obd data available, serial error or any error more deep in the stack)
 */
int viface_recv(final VehicleIFace* iface);

void viface_fill_infos_from_vin(final VehicleIFace * iface);

void viface_clear_data(final VehicleIFace* iface);
/**
 * A one time process to discover ECUs on the bus, and specific vehicle data
 */
void viface_discover_vehicle(VehicleIFace* iface);
void viface_dump(final VehicleIFace * iface);
/**
 * Messages filtering capabilities - add a filter
 * @param address CAN address (11bits, 29bits), OBD SA
 */
void viface_recv_filter_add(final VehicleIFace* iface, final Buffer * address);
/**
 * Messages filtering capabilities - remove a filter
 * @param address CAN address (11bits, 29bits), OBD SA
 */
bool viface_recv_filter_rm(final VehicleIFace* iface, final Buffer * address);
/**
 * Messages filtering capabilities - clear filters
 */
void viface_recv_filter_clear(final VehicleIFace* iface);

#endif