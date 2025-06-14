#include "libautodiag/com/obd/obd.h"

bool obd_standard_parse_buffer(final Vehicle* vehicle, final Buffer* bin_buffer) {
    if ( 2 < bin_buffer->size ) {
        byte priority = buffer_extract_0(bin_buffer); 
        byte receiver_address = buffer_extract_0(bin_buffer); 
        byte emitter_address = buffer_extract_0(bin_buffer); 
        
        final Buffer * address = buffer_new(); 
        buffer_append_bytes(address,&emitter_address,1); 
        final ECU* ecu = vehicle_ecu_add_if_not_in(vehicle, address->buffer, address->size); 
        buffer_free(address); 
        
        BufferList_append(ecu->obd_data_buffer,buffer_copy(bin_buffer)); 
        return true;
    } else { 
        log_msg(LOG_DEBUG, "Not enough data received"); 
        return false;
    } 
}

void obd_close(final OBDIFace* iface) {
    iface->device->close(iface->device);    
}

int obd_send(final OBDIFace* iface, const char *request) {
    return iface->device->send(iface->device, request);
}

int obd_recv(final OBDIFace* iface) {
    iface->device->clear_data(iface->device);
    final int initial_data_buffer_received = iface->vehicle->obd_data_buffer->size;
    switch(iface->device->recv(iface->device)) {
        case DEVICE_RECV_DATA: {
            if ( iface->device->parse_data == null ) {
                log_msg(LOG_WARNING, "parsing incoming data on a device without obd parser");
            } else {
                iface->device->parse_data(iface->device,iface->vehicle);
            }
        } break;
        case DEVICE_RECV_DATA_UNAVAILABLE: return OBD_RECV_NO_DATA;
        default: return OBD_RECV_ERROR;
    } 

    log_msg(LOG_DEBUG, "Storing in the corresponding service");
    Vehicle * v = iface->vehicle;
    if ( 0 < v->ecus_len ) {
        for(int i = 0; i < v->ecus_len; i++) {
            ECU * ecu = v->ecus[i];
            vehicle_ecu_empty_duplicated_info(ecu);

            for(int j = 0; j < ecu->obd_data_buffer->size; j++) {
                final Buffer * data = ecu->obd_data_buffer->list[j];
                if ( 0 < data->size ) {
                    final byte service_id = data->buffer[0];
                    if ( service_id == OBD_DIAGNOSTIC_SERVICE_NEGATIVE_RESPONSE ) {
                        log_msg(LOG_DEBUG, "Diagnostic Service negative code found (cannot response to the request)");
                        continue;
                    } else {
                        byte sid = service_id & ~OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE;
                        final Buffer * data_copy = buffer_copy(data);
                        buffer_extract_0(data_copy);
                        switch(sid) {
                            case OBD_SERVICE_SHOW_CURRENT_DATA: BufferList_append(ecu->obd_service.current_data, data_copy); break;
                            case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA: BufferList_append(ecu->obd_service.freeze_frame_data, data_copy); break;
                            case OBD_SERVICE_TESTS_RESULTS: BufferList_append(ecu->obd_service.tests_results, data_copy); break;
                            case OBD_SERVICE_TESTS_RESULTS_OTHER: BufferList_append(ecu->obd_service.tests_results_other, data_copy); break;
                            case OBD_SERVICE_CONTROL_OPERATION: BufferList_append(ecu->obd_service.control_operation, data_copy); break;
                            case OBD_SERVICE_PENDING_DTC: BufferList_append(ecu->obd_service.pending_dtc, data_copy); break;
                            case OBD_SERVICE_NONE: BufferList_append(ecu->obd_service.none, data_copy); break;
                            case OBD_SERVICE_SHOW_DTC: BufferList_append(ecu->obd_service.current_dtc, data_copy); break;
                            case OBD_SERVICE_CLEAR_DTC: BufferList_append(ecu->obd_service.clear_dtc, data_copy); break;
                            case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION: BufferList_append(ecu->obd_service.request_vehicle_information, data_copy); break;
                            case OBD_SERVICE_PERMANENT_DTC: BufferList_append(ecu->obd_service.permanent_dtc, data_copy); break;
                        }
                    }
                } else {
                    BufferList_remove_at(ecu->obd_data_buffer,j);
                    j--;
                }
            }
        }
    }
    vehicle_fill_global_obd_data_from_ecus(v);
    return iface->vehicle->obd_data_buffer->size - initial_data_buffer_received;
}

void obd_lock(final nonnull OBDIFace* iface) {
    iface->device->lock(iface->device);
}

void obd_unlock(final nonnull OBDIFace* iface) {
    iface->device->unlock(iface->device);
}

void obd_clear_data(final OBDIFace* iface) {
    iface->device->clear_data(iface->device);
    for ( int i = 0; i < iface->vehicle->ecus_len; i ++) {
        vehicle_ecu_empty(iface->vehicle->ecus[i]);
    }
    BufferList_empty(iface->vehicle->obd_data_buffer);
}

void obd_free(final OBDIFace* iface) {
    assert(iface != null);
    if ( iface->vehicle != null ) {
        vehicle_free(iface->vehicle);
    }
    if ( iface->device != null ) {
        log_msg(LOG_ERROR, "should free the device");
    }
    free(iface);
}

OBDIFace* obd_new() {
    final OBDIFace* iface = (OBDIFace*)malloc(sizeof(OBDIFace));
    iface->device = null;
    iface->vehicle = vehicle_new();
    return iface;
}

OBDIFace* obd_new_from_device(final nonnull Device* device) {
    assert(device != null);
    final nonnull OBDIFace* iface = obd_new();
    iface->device = device;
    return iface;
}

void obd_discover_vehicle(OBDIFace* iface) {
    saej1979_data_is_pid_supported(iface, false, 0x01);

    for(int i = 0; i < iface->vehicle->ecus_len; i++) {
        ECU* ecu = iface->vehicle->ecus[i];
        if ( ecu->name != null ) {
            free(ecu->name);
            ecu->name = null;
        }
    }
    saej1979_vehicle_info_discover_ecus_name(iface);
    saej1979_vehicle_info_discover_vin(iface);
}

OBDIFace* obd_open_from_device(final Device* device) {
    Serial * serial = (Serial*)device;
    OBDIFace * result = elm_open_from_serial(serial);
    if ( result == null ) {
        log_msg(LOG_ERROR, "Cannot open OBD interface from serial port %s", serial->location);
        return null;
    }
    obd_discover_vehicle(result);
    return result;
}