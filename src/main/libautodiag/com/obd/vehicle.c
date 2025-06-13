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
    vehicle_ecu_empty_duplicated_info(ecu);
}
void vehicle_ecu_empty_duplicated_info(ECU* ecu) {
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
void vehicle_ecu_debug(final ECU *ecu) {
    printf("ECU debug: {\n");
    printf("  address: %p\n", ecu->address);
    printf("  name: %s\n", ecu->name ? ecu->name : "null");
    printf("  obd_data_buffer: %p\n", ecu->obd_data_buffer);
    printf("  obd_service: {\n");
    printf("    current_data: %p\n", ecu->obd_service.current_data);
    printf("    freeze_frame_data: %p\n", ecu->obd_service.freeze_frame_data);
    printf("    tests_results: %p\n", ecu->obd_service.tests_results);
    printf("    tests_results_other: %p\n", ecu->obd_service.tests_results_other);
    printf("    control_operation: %p\n", ecu->obd_service.control_operation);
    printf("    pending_dtc: %p\n", ecu->obd_service.pending_dtc);
    printf("    none: %p\n", ecu->obd_service.none);
    printf("    current_dtc: %p\n", ecu->obd_service.current_dtc);
    printf("    clear_dtc: %p\n", ecu->obd_service.clear_dtc);
    printf("    request_vehicle_information: %p\n", ecu->obd_service.request_vehicle_information);
    printf("    permanent_dtc: %p\n", ecu->obd_service.permanent_dtc);
    printf("  }\n");
    printf("}\n");
}

Vehicle * vehicle_new() {
    Vehicle * v = (Vehicle *)malloc(sizeof(Vehicle));
    v->ecus = null;
    v->ecus_len = 0;
    v->obd_data_buffer = BufferList_new();
    v->brand = null;
    v->engine = null;
    v->internal.directory = null;
    return v;
}

void vehicle_free(Vehicle * v) {
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
        if ( v->brand != null ) {
            free(v->brand);
            v->brand = null;
        }
        if ( v->engine != null ) {
            free(v->engine);
            v->engine = null;
        }
        if ( v->internal.directory != null ) {
            free(v->internal.directory);
            v->internal.directory = null;
        }
    }
}

void vehicle_fill_global_obd_data_from_ecus(Vehicle* v) {
    BufferList_empty(v->obd_data_buffer);
    for ( int i = 0; i < v->ecus_len; i++) {
        ECU* ecu = v->ecus[i];
        for(int j = 0; j < ecu->obd_data_buffer->size; j++) {
            BufferList_append(v->obd_data_buffer,buffer_copy(ecu->obd_data_buffer->list[j]));
        }
    }
}
void vehicle_debug(Vehicle* v) {
    printf("Vehicle debug: {\n");
    printf("  ecus: %p\n", v->ecus);
    printf("  ecus_len: %d\n", v->ecus_len);
    for(int i = 0; i < v->ecus_len; i++) {
        ECU * ecu = v->ecus[i];
        vehicle_ecu_debug(ecu);
    }
    printf("  obd_data_buffer: %p\n", v->obd_data_buffer);
    printf("}\n");
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

LIST_DEFINE_MEMBERS_SYM(VehicleList,Vehicle)
