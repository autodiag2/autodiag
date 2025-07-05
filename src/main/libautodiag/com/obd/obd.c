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
        
        list_Buffer_append(ecu->data_buffer,buffer_copy(bin_buffer)); 
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
    final int initial_data_buffer_received = iface->vehicle->data_buffer->size;
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

            for(int j = 0; j < ecu->data_buffer->size; j++) {
                final Buffer * data = ecu->data_buffer->list[j];
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
    vehicle_fill_global_obd_data_from_ecus(v);
    return iface->vehicle->data_buffer->size - initial_data_buffer_received;
}

void obd_lock(final OBDIFace* iface) {
    iface->device->lock(iface->device);
}

void obd_unlock(final OBDIFace* iface) {
    iface->device->unlock(iface->device);
}

void obd_clear_data(final OBDIFace* iface) {
    iface->device->clear_data(iface->device);
    for ( int i = 0; i < iface->vehicle->ecus_len; i ++) {
        vehicle_ecu_empty(iface->vehicle->ecus[i]);
    }
    list_Buffer_empty(iface->vehicle->data_buffer);
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
void obd_fill_infos_from_vin(final OBDIFace * iface) {
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
    obd_fill_infos_from_vin(iface);
}

OBDIFace* obd_open_from_device(final Device* device) {
    Serial * serial = (Serial*)device;
    ELMDevice * elm = elm_open_from_serial(serial);
    if ( elm == null ) {
        log_msg(LOG_ERROR, "Cannot open OBD interface from serial port %s: device config has failed", serial->location);
        return null;
    }

    final OBDIFace* iface = obd_new();
    iface->device = CAST_DEVICE(elm);
    obd_discover_vehicle(iface);
    return iface;
}