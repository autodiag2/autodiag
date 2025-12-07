#include "libautodiag/com/vehicle_interface.h"
#include "libautodiag/com/serial/elm/elm.h"
#include "libautodiag/com/uds/uds.h"

void viface_recorder_reset(final VehicleIFace* iface) {
    record_clear();
}
list_object_Record * viface_recorder_get(final VehicleIFace* iface) {
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
    final Buffer * binRequest = buffer_from_ascii_hex(request);
    if ( binRequest == null || binRequest->size == 0 ) {
        log_msg(LOG_WARNING, "Sending at command through the vehicle interface is a bad pratice");
        return iface->device->send(iface->device, request);
    }
    final int result = viface_send(iface, binRequest);
    buffer_free(binRequest);
    return result;
}
int viface_send(final VehicleIFace* iface, final Buffer * binRequest) {
    char * request = buffer_to_hex_string(binRequest);
    final int result = iface->device->send(iface->device, request);
    free(request);
    viface_event_emit_on_request(iface, binRequest);
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
    log_msg(LOG_DEBUG, "VIFace dump state=%s", iface->state == VIFaceState_READY ? "ready" : "not ready");
    vehicle_dump(iface->vehicle);
}

VehicleIFace* viface_new() {
    final VehicleIFace* iface = (VehicleIFace*)malloc(sizeof(VehicleIFace));
    iface->device = null;
    iface->vehicle = vehicle_new();
    iface->state = VIFaceState_NOT_READY;
    iface->uds_enabled = false;
    iface->uds_tester_present_timer = null;
    iface->internal.onRequest = ehh_new();
    iface->internal.onResponse = ehh_new();
    return iface;
}

void viface_lock(final VehicleIFace* iface) {
    iface->device->lock(iface->device);
}

void viface_unlock(final VehicleIFace* iface) {
    iface->device->unlock(iface->device);
}

static void viface_open_abort(final VehicleIFace * iface) {
    iface->state = VIFaceState_NOT_READY;
    iface->uds_enabled = false;
    uds_viface_stop_tester_present_timer(iface);
}
bool viface_open_from_iface_device(final VehicleIFace * iface, final Device* device) {
    assert(iface != null);
    assert(device->type != null);
    uds_viface_stop_tester_present_timer(iface);
    if ( strcmp(device->type, "serial") == 0 ) {
        Serial * serial = (Serial*)device;
        if ( serial->status != SERIAL_STATE_READY ) {
            if ( serial_open(serial) == GENERIC_FUNCTION_ERROR ) {
                log_msg(LOG_ERROR, "Error while openning serial port");
                viface_open_abort(iface);
                return false;
            }
        }
        serial_lock(serial);
        ELMDevice * elm = elm_open_from_serial(serial);
        if ( elm == null ) {
            log_msg(LOG_ERROR, "Cannot open ELM interface from serial port %s: device config has failed", serial->location);
            serial_unlock(serial);
            viface_open_abort(iface);
            return false;
        }
        iface->device = CAST_DEVICE(elm);
        iface->device->unlock(CAST_DEVICE(iface->device));
    } else {
        log_msg(LOG_ERROR, "Unknown device type: %s aborting", device->type);
        viface_open_abort(iface);
        return false;
    }

    viface_discover_vehicle(iface);
    iface->state = VIFaceState_READY;
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
    if ( ! list_Buffer_find(iface->vehicle->internal.filter, address) ) {
        viface_lock(iface);
        final Buffer * address_copy = buffer_copy(address);
        list_Buffer_append(iface->vehicle->internal.filter, address_copy);
        update_filters_on_device(iface);
        vehicle_event_emit_on_filter_change_add(iface->vehicle, address_copy);
        viface_unlock(iface);
    }
}
bool viface_recv_filter_rm(final VehicleIFace* iface, final Buffer * address) {
    final Buffer * found = list_Buffer_find(iface->vehicle->internal.filter, address);
    if ( found ) {
        viface_lock(iface);
        list_Buffer_remove(iface->vehicle->internal.filter, found);
        update_filters_on_device(iface);
        vehicle_event_emit_on_filter_change_rm(iface->vehicle, address);
        buffer_free(found);
        viface_unlock(iface);
    }
    return found != null;
}
void viface_recv_filter_clear(final VehicleIFace* iface) {
    viface_lock(iface);
    list_Buffer_empty(iface->vehicle->internal.filter);
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
        for(unsigned int i = 0; i < v->ecus_len; i++) {
            ECU * ecu = v->ecus[i];
            vehicle_ecu_empty_duplicated_info(ecu);
            if ( 0 < v->internal.filter->size && ! list_Buffer_contains(v->internal.filter, ecu->address) ) {
                list_Buffer_empty(ecu->data_buffer);
            } else {
                for(unsigned int j = 0; j < ecu->data_buffer->size; j++) {
                    final Buffer * data = ecu->data_buffer->list[j];
                    if ( 0 < data->size ) {
                        viface_event_emit_on_response(iface, ecu, data);
                        byte service_id = data->buffer[0];
                        if ( service_id == OBD_DIAGNOSTIC_SERVICE_NEGATIVE_RESPONSE ) {
                            log_msg(LOG_DEBUG, "Diagnostic Service negative code found (cannot response to the request)");
                            continue;
                        } else {
                            service_id &= ~OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE;
                            final Buffer * data_copy = buffer_copy(data);
                            buffer_extract_0(data_copy);
                            switch(service_id) {
                                case OBD_SERVICE_SHOW_CURRENT_DATA: list_Buffer_append(ecu->obd_service.current_data, data_copy); break;
                                case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA: list_Buffer_append(ecu->obd_service.freeze_frame_data, data_copy); break;
                                case OBD_SERVICE_TESTS_RESULTS: list_Buffer_append(ecu->obd_service.tests_results, data_copy); break;
                                case OBD_SERVICE_TESTS_RESULTS_OTHER: list_Buffer_append(ecu->obd_service.tests_results_other, data_copy); break;
                                case OBD_SERVICE_CONTROL_OPERATION: list_Buffer_append(ecu->obd_service.control_operation, data_copy); break;
                                case OBD_SERVICE_PENDING_DTC: list_Buffer_append(ecu->obd_service.pending_dtc, data_copy); break;
                                case OBD_SERVICE_NONE: list_Buffer_append(ecu->obd_service.none, data_copy); break;
                                case OBD_SERVICE_SHOW_DTC: list_Buffer_append(ecu->obd_service.current_dtc, data_copy); break;
                                case OBD_SERVICE_CLEAR_DTC: list_Buffer_append(ecu->obd_service.clear_dtc, data_copy); break;
                                case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION: list_Buffer_append(ecu->obd_service.request_vehicle_information, data_copy); break;
                                case OBD_SERVICE_PERMANENT_DTC: list_Buffer_append(ecu->obd_service.permanent_dtc, data_copy); break;
                            }
                        }
                    } else {
                        list_Buffer_remove_at(ecu->data_buffer,j);
                        j--;
                    }
                }
            }
        }
    }
    vehicle_fill_global_data_buffer_from_ecus(v);
    return iface->vehicle->data_buffer->size - initial_data_buffer_received;
}


void viface_clear_data(final VehicleIFace* iface) {
    iface->device->clear_data(iface->device);
    for ( int i = 0; i < iface->vehicle->ecus_len; i ++) {
        vehicle_ecu_empty(iface->vehicle->ecus[i]);
    }
    list_Buffer_empty(iface->vehicle->data_buffer);
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
    iface->uds_enabled = uds_is_enabled(iface);
    saej1979_vehicle_info_discover_vin(iface);
    if ( iface->vehicle->vin == null ) {
        iface->vehicle->vin = buffer_new();
    }
    if ( iface->uds_enabled && iface->vehicle->vin->size == 0 ) {
        final list_Buffer * result = uds_read_data_by_identifier(iface, UDS_DID_VIN);
        assert(result->size <= 1);
        if ( 0 < result->size ) {
            iface->vehicle->vin = buffer_copy(result->list[0]);
        }
    }
    if ( 0 < iface->vehicle->vin->size ) {
        viface_fill_infos_from_vin(iface);
    }
}