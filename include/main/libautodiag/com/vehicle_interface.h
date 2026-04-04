#ifndef __AD_VEHICLE_INTERFACE_H
#define __AD_VEHICLE_INTERFACE_H

#include <stdarg.h>
#include "libautodiag/com/device.h"
#include "libautodiag/model/vehicle.h"
#include "libautodiag/com/recorder.h"
#include "libautodiag/com/vehicle_signal.h"

typedef enum {
    VIFaceState_READY,
    VIFaceState_NOT_READY,
    VIFaceState_RECONNECTING,
} VIFaceState;

typedef struct VehicleIFace {
    /**
     * Device onto which communication should start.
     */
    Device* device;
    /**
     * Data structure representing the data received from the vehicle itself.
     */
    Vehicle * vehicle;
    struct {
        /**
         * Does have uds on the current connexion.
         */
        bool enabled;
        /**
         * A thread that send periodically TesterPresent messages.
         */
        pthread_t * tester_present_timer;
    } uds;

    struct {
        struct {
            bool enabled;
            uint64_t last_activity_ms;
            uint64_t activity_threshold_ms;
            int activity_poll_ms;
            pthread_t * activity_thread;
            void (*disable)(struct VehicleIFace * iface);
            void (*update_last_activity)(struct VehicleIFace * iface);
            bool (*probe)(struct VehicleIFace * iface);
            bool (*should_probe)(struct VehicleIFace * iface);
            bool (*start)(struct VehicleIFace * iface);
            bool (*stop)(struct VehicleIFace * iface);
        } checking;
        /**
         * State of this interface, whether ready to communicate with the vehicle or not.
         */
        VIFaceState _state;
        EventHandlerHolder * onConnectionStateChanged;
        void (*set_state)(struct VehicleIFace * iface, VIFaceState state);
    } connection;

    void (*lock)(struct VehicleIFace * iface);
    void (*unlock)(struct VehicleIFace * iface);
    /**
     * @return number of bytes sent or DEVICE_ERROR
     */
    int (*send)(struct VehicleIFace * iface, Buffer * request);
    /**
     * @return number of data buffer received, negative on error
     */
    int (*recv)(struct VehicleIFace * iface);
    /**
     * clear any data received
     */
    void (*clear_data)(struct VehicleIFace * iface);
        
    struct {
        EventHandlerHolder * onRequest;
        EventHandlerHolder * onResponse;
    } internal;

} VehicleIFace;

#define AD_VIFACE_CONNECTION_CHECKING_ACTIVITY_THRESHOLD_MS 2000
#define AD_VIFACE_CONNECTION_CHECKING_ACTIVITY_POLL_MS 1000

#define ad_viface_event_emit_on_request(iface, binRequest) \
    ehh_trigger(iface->internal.onRequest, (void(*)(Buffer*)), binRequest);

#define ad_viface_event_emit_on_response(iface, ecu, binResponse) \
    ehh_trigger(iface->internal.onResponse, (void(*)(ad_object_ECU*,Buffer*)), ecu, binResponse);

void viface_recorder_set_state(final VehicleIFace* iface, final bool state);
void viface_recorder_reset(final VehicleIFace* iface);
ad_list_ad_object_Record * viface_recorder_get(final VehicleIFace* iface);

VehicleIFace* viface_new();
void viface_free(final VehicleIFace* iface);
void viface_lock(final VehicleIFace *iface);
void viface_unlock(final VehicleIFace *iface);
void viface_close(final VehicleIFace* iface);
/**
 * Send some data on the wire(less), without taking care of device implementation details
 * or communication protocol headers
 * @return number of bytes sent or DEVICE_ERROR
 */
int viface_send(final VehicleIFace * iface, final Buffer * binRequest);
/**
 * @return number of bytes sent or DEVICE_ERROR
 */
int viface_send_str(final VehicleIFace * iface, final char * request);
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
 * @param address ecu to target (CAN address (11bits, 29bits), OBD SA)
 */
void viface_recv_filter_add(final VehicleIFace* iface, final Buffer * address);
/**
 * Messages filtering capabilities - remove a filter
 * @param address ecu to target (CAN address (11bits, 29bits), OBD SA)
 */
bool viface_recv_filter_rm(final VehicleIFace* iface, final Buffer * address);
/**
 * Messages filtering capabilities - clear filters
 */
void viface_recv_filter_clear(final VehicleIFace* iface);
/**
 * Send a signal over the transport layer (ISO-TP, DOIP, KWP2000), parse and reply a signal.
 * @param signal to send and receive
 * @param ... parameters of the signal (eg. frame number)
 * @return true if signal received and parsed else on any error false
 */
bool viface_use_signal(final VehicleIFace *iface, ad_object_vehicle_signal *signal, double *result_rv, ...);
/**
 * No matter how many ECU have responded, use the first response.
 */
Buffer * ad_viface_find_first_response(final VehicleIFace * iface, byte service_id);

#endif