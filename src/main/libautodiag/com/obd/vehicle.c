#include "libautodiag/com/obd/vehicle.h"

ECU* vehicle_ecu_add_if_not_in(Vehicle* v, byte* address, int size) {
    for(int i = 0; i < v->ecus_len; i++) {
        ECU* ecu = v->ecus[i];
        if ( size == ecu->address->size ) {
            if ( memcmp(ecu->address->buffer,address, size) == 0 ) {
                return ecu;
            }        
        }
    }
    log_msg(LOG_DEBUG, "New ECU added");
    return vehicle_ecu_add(v,address,size);
}

ECU* vehicle_ecu_add(Vehicle* v, byte* address, int size) {
    final ECU* ecu = vehicle_ecu_new();
    buffer_append_bytes(ecu->address,address,size);
    v->ecus = (ECU**)realloc(v->ecus,sizeof(ECU*)*(++v->ecus_len));
    v->ecus[v->ecus_len-1] = ecu;
    return ecu;
}

ECU* vehicle_ecu_new() {
    ECU* ecu = (ECU*)malloc(sizeof(ECU));
    ecu->address = buffer_new();
    ecu->name = null;
    ecu->obd_data_buffer = BufferList_new();
    ecu->obd_service.current_data = BufferList_new();
    ecu->obd_service.freeze_frame_data = BufferList_new();
    ecu->obd_service.tests_results = BufferList_new();
    ecu->obd_service.tests_results_other = BufferList_new();
    ecu->obd_service.control_operation = BufferList_new();
    ecu->obd_service.pending_dtc = BufferList_new();
    ecu->obd_service.none = BufferList_new();
    ecu->obd_service.current_dtc = BufferList_new();
    ecu->obd_service.clear_dtc = BufferList_new();
    ecu->obd_service.request_vehicle_information = BufferList_new();
    ecu->obd_service.permanent_dtc = BufferList_new();
    return ecu;
}

void vehicle_ecu_empty(ECU* ecu) {
    BufferList_empty(ecu->obd_data_buffer);
    BufferList_empty(ecu->obd_service.current_data);
    BufferList_empty(ecu->obd_service.freeze_frame_data);
    BufferList_empty(ecu->obd_service.tests_results);
    BufferList_empty(ecu->obd_service.tests_results_other);
    BufferList_empty(ecu->obd_service.control_operation);
    BufferList_empty(ecu->obd_service.pending_dtc);
    BufferList_empty(ecu->obd_service.none);
    BufferList_empty(ecu->obd_service.current_dtc);
    BufferList_empty(ecu->obd_service.clear_dtc);
    BufferList_empty(ecu->obd_service.request_vehicle_information);
    BufferList_empty(ecu->obd_service.permanent_dtc);
}
void vehicle_ecu_free(ECU* ecu) {
    BufferList_free(ecu->obd_data_buffer);
    if ( ecu->address != null ) {
        free(ecu->address);
        ecu->address = null;
    }
    if ( ecu->name != null ) {
        free(ecu->name);
        ecu->name = null;
    }
}

VEHICLE vehicle_new() {
    VEHICLE v = (VEHICLE)malloc(sizeof(Vehicle));
    v->ecus = null;
    v->ecus_len = 0;
    v->obd_data_buffer = BufferList_new();
    return v;
}

void vehicle_free(VEHICLE v) {
    if ( v != null ) {
        BufferList_free(v->obd_data_buffer);
        if ( v->ecus != null ) {
            for(int i = 0; i < v->ecus_len; i++) {
                vehicle_ecu_free(v->ecus[i]);
            }
            free(v->ecus);
            v->ecus = null;
        }
        v->ecus_len = 0;
    }
}

void vehicle_fill_global_obd_data_from_ecus(Vehicle* v) {
    for ( int i = 0; i < v->ecus_len; i++) {
        ECU* ecu = v->ecus[i];
        for(int j = 0; j < ecu->obd_data_buffer->size; j++) {
            BufferList_append(v->obd_data_buffer,buffer_copy(ecu->obd_data_buffer->list[j]));
        }
    }
}

void vehicle_dump(Vehicle* v) {
    log_msg(LOG_DEBUG, "Vehicle dump");
    log_msg(LOG_DEBUG, "Global data");
    BufferList_dump(v->obd_data_buffer);
    for(int i = 0; i < v->ecus_len; i++) {
        log_msg(LOG_DEBUG, "Dump of one ECU");
        ECU * ecu = v->ecus[i];
        log_msg(LOG_DEBUG, "Address");
        Buffer * address = ecu->address;
        buffer_dump(address);
        log_msg(LOG_DEBUG, "content");
        BufferList_dump(ecu->obd_data_buffer);

        log_msg(LOG_DEBUG, "in current_data");
        BufferList_dump(ecu->obd_service.current_data);
        log_msg(LOG_DEBUG, "in freeze_frame_data");
        BufferList_dump(ecu->obd_service.freeze_frame_data);
        log_msg(LOG_DEBUG, "in tests_results");
        BufferList_dump(ecu->obd_service.tests_results);
        log_msg(LOG_DEBUG, "in tests_results_other");
        BufferList_dump(ecu->obd_service.tests_results_other);
        log_msg(LOG_DEBUG, "in control_operation");
        BufferList_dump(ecu->obd_service.control_operation);
        log_msg(LOG_DEBUG, "in pending_dtc");
        BufferList_dump(ecu->obd_service.pending_dtc);
        log_msg(LOG_DEBUG, "in none");
        BufferList_dump(ecu->obd_service.none);
        log_msg(LOG_DEBUG, "in current_dtc");
        BufferList_dump(ecu->obd_service.current_dtc);
        log_msg(LOG_DEBUG, "in clear_dtc");
        BufferList_dump(ecu->obd_service.clear_dtc);
        log_msg(LOG_DEBUG, "in request_vehicle_information");
        BufferList_dump(ecu->obd_service.request_vehicle_information);
        log_msg(LOG_DEBUG, "in permanent_dtc");
        BufferList_dump(ecu->obd_service.permanent_dtc);
    
    }
}

