#include "libautodiag/com/vehicle_interface.h"
#include "libautodiag/com/serial/elm/elm.h"
#include "libautodiag/com/uds/uds.h"
#include "libautodiag/com/doip/doip.h"

void viface_recorder_reset(final VehicleIFace* iface) {
    record_clear();
}
ad_list_ad_object_Record * viface_recorder_get(final VehicleIFace* iface) {
    return recorder_get();
}
void viface_recorder_set_state(final VehicleIFace* iface, final bool state) {
    if ( state ) {
        ehh_register(iface->internal.onRequest, record_on_request);
        ehh_register(iface->internal.onResponse, record_on_response);
    } else {
        ehh_unregister(iface->internal.onRequest, record_on_request);
        ehh_unregister(iface->internal.onResponse, record_on_response);
    }
}
void viface_close(final VehicleIFace* iface) {
    uds_viface_stop_tester_present_timer(iface);
    if ( iface->device != null ) {
        iface->device->close(iface->device);
    }
}
int viface_send_str(final VehicleIFace * iface, final char * request) {
    final Buffer * binRequest = ad_buffer_from_ascii_hex(request);
    if ( binRequest == null || binRequest->size == 0 ) {
        log_msg(LOG_WARNING, "Sending at command through the vehicle interface is a bad pratice");
        return iface->device->send(iface->device, request);
    }
    final int result = viface_send(iface, binRequest);
    ad_buffer_free(binRequest);
    return result;
}
int viface_send(final VehicleIFace* iface, final Buffer * binRequest) {
    char * request = ad_buffer_to_hex_string(binRequest);
    final int result = iface->device->send(iface->device, request);
    free(request);
    ad_viface_event_emit_on_request(iface, binRequest);
    return result;
}
void viface_free(final VehicleIFace* iface) {
    assert(iface != null);
    if ( iface->vehicle != null ) {
        vehicle_free(iface->vehicle);
    }
    if ( iface->device != null ) {
        log_msg(LOG_ERROR, "should free the device");
    }
    free(iface);
}
void viface_dump(final VehicleIFace * iface) {
    log_msg(LOG_DEBUG, "VIFace dump state=%s", iface->connection._state == VIFaceState_READY ? "ready" : "not ready");
    vehicle_dump(iface->vehicle);
}
static void connection_set_state(VehicleIFace * iface, VIFaceState state) {
    if ( iface->connection._state != state ) {
        iface->connection._state = state;
        ehh_trigger(iface->connection.onConnectionStateChanged, (void(*)(VehicleIFace*, VIFaceState)), iface, state);
    }
}
static void connection_checking_update_last_activity(VehicleIFace * iface) {
    assert( iface != null );
    iface->connection.checking.last_activity_ms = time_ms();
}
static void connection_checking_disable(VehicleIFace * iface) {
    iface->connection.checking.enabled = false;
    iface->connection.checking.stop(iface);
}
static bool connection_checking_should_probe(VehicleIFace * iface) {
    return ( iface->connection.checking.activity_threshold_ms <= (iface->connection.checking.last_activity_ms - time_ms()) );
}
static bool connection_checking_probe(VehicleIFace * iface) {
    assert( iface != null );
    if ( ! iface->connection.checking.enabled ) {
        log_msg(LOG_DEBUG, "probing is disabled");
        return false;
    }

    log_msg(LOG_DEBUG, "Sending a probe");

    if ( iface->uds.enabled ) {
        if ( uds_tester_present(iface, true) ) {
            iface->connection.checking.update_last_activity(iface);
            log_msg(LOG_DEBUG, "Probe detected connection alive");
            connection_set_state(iface, VIFaceState_READY);
            return true;
        }
    } else {
        Buffer * request = ad_buffer_from_ints(OBD_SERVICE_SHOW_CURRENT_DATA, 0x00);
        iface->lock(iface);
        if ( iface->send(iface, request) == DEVICE_ERROR ) {
            goto dead_connection;
        }
        iface->clear_data(iface);
        if ( iface->recv(iface) <= 0 ) {
            goto dead_connection;
        }
        iface->connection.checking.update_last_activity(iface);
        iface->unlock(iface);
        log_msg(LOG_DEBUG, "Probe detected connection alive");
        connection_set_state(iface, VIFaceState_READY);
        return true;
        
        dead_connection:
            iface->unlock(iface);
    }
    log_msg(LOG_WARNING, "Connection seem dead");
    connection_set_state(iface, VIFaceState_NOT_READY);
    return false;
}
static void * connection_checking_activity_loop(void * arg) {
    log_msg(LOG_DEBUG, "Starting connection checking loop");
    VehicleIFace * iface = (VehicleIFace*)arg;
    while ( iface->connection.checking.activity_thread != null ) {
        if ( iface->connection.checking.should_probe(iface) ) {
            if ( iface->connection.checking.probe(iface) ) {
                iface->connection.checking.update_last_activity(iface);
            }
        }
        usleep(iface->connection.checking.activity_poll_ms * 1000);
    }
    return null;
}
static bool connection_checking_start(VehicleIFace * iface) {
    log_msg(LOG_DEBUG, "Starting connection checking");
    if ( iface->connection.checking.activity_thread == null ) {
        iface->connection.checking.activity_thread = (pthread_t*)malloc(sizeof(pthread_t));
        if ( pthread_create(iface->connection.checking.activity_thread, null, connection_checking_activity_loop, (void*)iface) == 0 ) {
            return true;
        } else {
            log_msg(LOG_ERROR, "thread creation");
        }
    }
    return false;
}
static bool connection_checking_stop(VehicleIFace * iface) {
    if ( iface->connection.checking.activity_thread != null ) {
        THREAD_CANCEL(iface->connection.checking.activity_thread);
        free(iface->connection.checking.activity_thread);
        iface->connection.checking.activity_thread = null;
        return true;
    }
    return false;
}
VehicleIFace* viface_new() {
    final VehicleIFace* iface = (VehicleIFace*)malloc(sizeof(VehicleIFace));
    iface->device = null;
    iface->vehicle = vehicle_new();
    iface->uds.enabled = false;
    iface->uds.tester_present_timer = null;
    iface->connection.checking.activity_threshold_ms = AD_VIFACE_CONNECTION_CHECKING_ACTIVITY_THRESHOLD_MS;
    iface->connection.checking.last_activity_ms = 0;
    iface->connection.checking.activity_thread = null;
    iface->connection.checking.probe = connection_checking_probe;
    iface->connection.checking.should_probe = connection_checking_should_probe;
    iface->connection.checking.update_last_activity = connection_checking_update_last_activity;
    iface->connection.checking.activity_poll_ms = AD_VIFACE_CONNECTION_CHECKING_ACTIVITY_POLL_MS;
    iface->connection.checking.start = connection_checking_start;
    iface->connection.checking.stop = connection_checking_stop;
    iface->connection.checking.disable = connection_checking_disable;
    iface->connection.checking.enabled = true;
    iface->connection.set_state = connection_set_state;
    iface->connection.onConnectionStateChanged = ehh_new();
    iface->connection._state = VIFaceState_NOT_READY;
    iface->recv = viface_recv;
    iface->send = viface_send;
    iface->lock = viface_lock;
    iface->unlock = viface_unlock;
    iface->clear_data = viface_clear_data;
    iface->internal.onRequest = ehh_new();
    iface->internal.onResponse = ehh_new();
    return iface;
}

void viface_lock(final VehicleIFace* iface) {
    assert(iface != null);
    assert(iface->device != null);
    iface->device->lock(iface->device);
}

void viface_unlock(final VehicleIFace* iface) {
    assert(iface != null);
    assert(iface->device != null);
    iface->device->unlock(iface->device);
}

static void viface_open_abort(final VehicleIFace * iface) {
    iface->connection.set_state(iface, VIFaceState_NOT_READY);
    iface->uds.enabled = false;
    uds_viface_stop_tester_present_timer(iface);
}
bool viface_open_from_iface_device(final VehicleIFace * iface, final Device* device) {
    assert(iface != null);
    uds_viface_stop_tester_present_timer(iface);
    iface->connection.checking.stop(iface);
    if ( device->state != AD_DEVICE_STATE_READY ) {
        if ( device->open(device) == DEVICE_ERROR ) {
            log_msg(LOG_ERROR, "Error while openning device");
            viface_open_abort(iface);
            return false;
        }
    }
    device->lock(AD_DEVICE(device));
    if ( device->type == AD_DEVICE_TYPE_AUTO ) {
        device->type = ad_device_type_from_location(device->location);
    }
    switch(device->type) {
        case AD_DEVICE_TYPE_SERIAL: {
            Serial * serial = (Serial*)device;
            ELMDevice * elm = elm_open_from_serial(serial);
            device->unlock(AD_DEVICE(device));
            if ( elm == null ) {
                log_msg(LOG_ERROR, "Cannot open ELM interface from serial port %s: device config has failed", serial->location);
                device->unlock(AD_DEVICE(device));
                viface_open_abort(iface);
                return false;
            }
            iface->device = AD_DEVICE(elm);
            elm->lock(AD_DEVICE(elm));
        } break;
        case AD_DEVICE_TYPE_DOIP: {
            ad_object_DoIPDevice * doip_device = (ad_object_DoIPDevice*)device;
            doip_configure(doip_device);
            iface->device = AD_DEVICE(doip_device);
        } break;
        default: {
            log_msg(LOG_ERROR, "Unknown device type: 0x%X aborting", device->type);
            device->unlock(AD_DEVICE(device));
            viface_open_abort(iface);
            return false;
        }
    }
    iface->device->unlock(AD_DEVICE(iface->device));
    viface_discover_vehicle(iface);
    iface->connection.set_state(iface, VIFaceState_READY);
    iface->connection.checking.start(iface);
    return true;
}
VehicleIFace* viface_open_from_device(final Device* device) {
    final VehicleIFace* iface = viface_new();
    if ( viface_open_from_iface_device (iface, device) ) {
        return iface;
    } else {
        viface_free(iface);
        return null;
    }
}
static bool update_filters_on_device(final VehicleIFace* iface) {
    if ( iface->device->set_filter_by_address != null ) {
        iface->device->set_filter_by_address(iface->device, iface->vehicle->internal.filter);
        return true;
    }
    return false;
}
void viface_recv_filter_add(final VehicleIFace* iface, final Buffer * address) {
    if ( ! ad_list_Buffer_find(iface->vehicle->internal.filter, address) ) {
        viface_lock(iface);
        final Buffer * address_copy = ad_buffer_copy(address);
        ad_list_Buffer_append(iface->vehicle->internal.filter, address_copy);
        update_filters_on_device(iface);
        vehicle_event_emit_on_filter_change_add(iface->vehicle, address_copy);
        viface_unlock(iface);
    }
}
bool viface_recv_filter_rm(final VehicleIFace* iface, final Buffer * address) {
    final Buffer * found = ad_list_Buffer_find(iface->vehicle->internal.filter, address);
    if ( found ) {
        viface_lock(iface);
        ad_list_Buffer_remove(iface->vehicle->internal.filter, found);
        update_filters_on_device(iface);
        vehicle_event_emit_on_filter_change_rm(iface->vehicle, address);
        ad_buffer_free(found);
        viface_unlock(iface);
    }
    return found != null;
}
void viface_recv_filter_clear(final VehicleIFace* iface) {
    viface_lock(iface);
    ad_list_Buffer_empty(iface->vehicle->internal.filter);
    update_filters_on_device(iface);
    vehicle_event_emit_on_filter_change_clear(iface->vehicle);
    viface_unlock(iface);
}
int viface_recv(final VehicleIFace* iface) {
    iface->device->clear_data(iface->device);
    final int initial_data_buffer_received = iface->vehicle->data_buffer->size;
    switch(iface->device->recv(iface->device)) {
        case DEVICE_RECV_DATA: {
            if ( iface->device->parse_data == null ) {
                log_msg(LOG_WARNING, "parsing incoming data on a device without obd parser");
            } else {
                iface->device->parse_data(iface->device,iface->vehicle);
            }
        } break;
        case DEVICE_RECV_DATA_UNAVAILABLE: return VIFACE_RECV_UNSUPPORTED;
        default: return VIFACE_RECV_ERROR;
    } 

    log_msg(LOG_DEBUG, "Storing in the corresponding service");
    Vehicle * v = iface->vehicle;
    if ( 0 < v->ecus_len ) {
        for(unsigned i = 0; i < v->ecus_len; i++) {
            ECU * ecu = v->ecus[i];
            vehicle_ecu_empty_duplicated_info(ecu);
            if ( 0 < v->internal.filter->size && ! ad_list_Buffer_contains(v->internal.filter, ecu->address) ) {
                ad_list_Buffer_empty(ecu->data_buffer);
            } else {
                for(unsigned j = 0; j < ecu->data_buffer->size; j++) {
                    final Buffer * data = ecu->data_buffer->list[j];
                    if ( 0 < data->size ) {
                        ad_viface_event_emit_on_response(iface, ecu, data);
                        byte service_id = data->buffer[0];
                        if ( service_id == OBD_DIAGNOSTIC_SERVICE_NEGATIVE_RESPONSE ) {
                            log_msg(LOG_DEBUG, "Diagnostic Service negative code found (cannot response to the request)");
                            continue;
                        } else {
                            service_id &= ~OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE;
                            final Buffer * data_copy = ad_buffer_copy(data);
                            ad_buffer_extract_0(data_copy);
                            switch(service_id) {
                                case OBD_SERVICE_SHOW_CURRENT_DATA: ad_list_Buffer_append(ecu->obd_service.current_data, data_copy); break;
                                case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA: ad_list_Buffer_append(ecu->obd_service.freeze_frame_data, data_copy); break;
                                case OBD_SERVICE_TESTS_RESULTS: ad_list_Buffer_append(ecu->obd_service.tests_results, data_copy); break;
                                case OBD_SERVICE_TESTS_RESULTS_OTHER: ad_list_Buffer_append(ecu->obd_service.tests_results_other, data_copy); break;
                                case OBD_SERVICE_CONTROL_OPERATION: ad_list_Buffer_append(ecu->obd_service.control_operation, data_copy); break;
                                case OBD_SERVICE_PENDING_DTC: ad_list_Buffer_append(ecu->obd_service.pending_dtc, data_copy); break;
                                case OBD_SERVICE_NONE: ad_list_Buffer_append(ecu->obd_service.none, data_copy); break;
                                case OBD_SERVICE_SHOW_DTC: ad_list_Buffer_append(ecu->obd_service.current_dtc, data_copy); break;
                                case OBD_SERVICE_CLEAR_DTC: ad_list_Buffer_append(ecu->obd_service.clear_dtc, data_copy); break;
                                case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION: ad_list_Buffer_append(ecu->obd_service.request_vehicle_information, data_copy); break;
                                case OBD_SERVICE_PERMANENT_DTC: ad_list_Buffer_append(ecu->obd_service.permanent_dtc, data_copy); break;
                                default:
                                    log_msg(LOG_WARNING, "Received data for unknown OBD service ID: 0x%02X", service_id);
                                    ad_buffer_free(data_copy);
                                    break;
                            }
                        }
                    } else {
                        ad_list_Buffer_remove_at(ecu->data_buffer,j);
                        j--;
                    }
                }
            }
        }
    }
    iface->connection.checking.update_last_activity(iface);
    vehicle_fill_global_data_buffer_from_ecus(v);
    return iface->vehicle->data_buffer->size - initial_data_buffer_received;
}


void viface_clear_data(final VehicleIFace* iface) {
    iface->device->clear_data(iface->device);
    for ( int i = 0; i < iface->vehicle->ecus_len; i ++) {
        vehicle_ecu_empty(iface->vehicle->ecus[i]);
    }
    ad_list_Buffer_empty(iface->vehicle->data_buffer);
}

void viface_fill_infos_from_vin(final VehicleIFace * iface) {
    if ( iface->vehicle->vin != null && 17 <= iface->vehicle->vin->size ) {
        final ISO3779 * decoder = ISO3779_new();
        ISO3779_decode(decoder, iface->vehicle->vin);
        if ( decoder->country != null ) {
            iface->vehicle->country = strdup(decoder->country);
        }
        if ( decoder->manufacturer != null ) {
            iface->vehicle->manufacturer = strdup(decoder->manufacturer);
        }
        iface->vehicle->year = decoder->year;
        ISO3779_free(decoder);
    }
}
void viface_discover_vehicle(VehicleIFace* iface) {
    saej1979_data_is_pid_supported(iface, false, 0x01);
    for(int i = 0; i < iface->vehicle->ecus_len; i++) {
        ECU* ecu = iface->vehicle->ecus[i];
        if ( ecu->name != null ) {
            free(ecu->name);
            ecu->name = null;
        }
    }
    saej1979_vehicle_info_discover_ecus_name(iface);
    iface->uds.enabled = uds_is_enabled(iface);
    saej1979_vehicle_info_discover_vin(iface);
    if ( iface->vehicle->vin == null ) {
        iface->vehicle->vin = ad_buffer_new();
    }
    if ( iface->uds.enabled && iface->vehicle->vin->size == 0 ) {
        final ad_list_Buffer * result = uds_read_data_by_identifier(iface, UDS_DID_VIN);
        assert(result->size <= 1);
        if ( 0 < result->size ) {
            iface->vehicle->vin = ad_buffer_copy(result->list[0]);
        }
    }
    if ( 0 < iface->vehicle->vin->size ) {
        if ( 17 != iface->vehicle->vin->size ) {
            log_msg(LOG_WARNING, "Non standard VIN detected: size=%d content=%s", iface->vehicle->vin->size, ad_buffer_to_ascii(iface->vehicle->vin));
        }
        // If the manufacturer return it in a non standard way
        ad_buffer_slice_non_alphanum(iface->vehicle->vin);
        viface_fill_infos_from_vin(iface);
    }
}