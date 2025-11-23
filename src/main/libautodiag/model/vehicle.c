#include "libautodiag/model/vehicle.h"

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
    vehicle_event_emit_on_ecu_register(v, ecu);
    return ecu;
}

ECU* vehicle_ecu_new() {
    ECU* ecu = (ECU*)malloc(sizeof(ECU));
    ecu->address = buffer_new();
    ecu->name = null;
    ecu->data_buffer = list_Buffer_new();
    ecu->obd_service.current_data = list_Buffer_new();
    ecu->obd_service.freeze_frame_data = list_Buffer_new();
    ecu->obd_service.tests_results = list_Buffer_new();
    ecu->obd_service.tests_results_other = list_Buffer_new();
    ecu->obd_service.control_operation = list_Buffer_new();
    ecu->obd_service.pending_dtc = list_Buffer_new();
    ecu->obd_service.none = list_Buffer_new();
    ecu->obd_service.current_dtc = list_Buffer_new();
    ecu->obd_service.clear_dtc = list_Buffer_new();
    ecu->obd_service.request_vehicle_information = list_Buffer_new();
    ecu->obd_service.permanent_dtc = list_Buffer_new();
    return ecu;
}
ECU* vehicle_search_ecu_by_address(Vehicle* v, Buffer* address) {
    for(int i = 0; i < v->ecus_len; i++) {
        if ( buffer_cmp(v->ecus[i]->address, address) == 0 ) {
            return v->ecus[i];
        }
    }
    return null;
}
void vehicle_ecu_empty(ECU* ecu) {
    list_Buffer_empty(ecu->data_buffer);
    vehicle_ecu_empty_duplicated_info(ecu);
}
void vehicle_ecu_empty_duplicated_info(ECU* ecu) {
    list_Buffer_empty(ecu->obd_service.current_data);
    list_Buffer_empty(ecu->obd_service.freeze_frame_data);
    list_Buffer_empty(ecu->obd_service.tests_results);
    list_Buffer_empty(ecu->obd_service.tests_results_other);
    list_Buffer_empty(ecu->obd_service.control_operation);
    list_Buffer_empty(ecu->obd_service.pending_dtc);
    list_Buffer_empty(ecu->obd_service.none);
    list_Buffer_empty(ecu->obd_service.current_dtc);
    list_Buffer_empty(ecu->obd_service.clear_dtc);
    list_Buffer_empty(ecu->obd_service.request_vehicle_information);
    list_Buffer_empty(ecu->obd_service.permanent_dtc);
}
void vehicle_ecu_free(ECU* ecu) {
    list_Buffer_free(ecu->data_buffer);
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
    printf("  data_buffer: %p\n", ecu->data_buffer);
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
    v->data_buffer = list_Buffer_new();
    v->country = null;
    v->manufacturer = null;
    v->year = VEHICLE_YEAR_EMPTY;
    v->engine = null;
    v->vin = null;
    v->internal.directory = null;
    v->internal.events.onECURegister = ehh_new();
    v->internal.events.onFilterChange = ehh_new();
    v->internal.filter = list_Buffer_new();
    return v;
}

void vehicle_free(Vehicle * v) {
    if ( v != null ) {
        list_Buffer_free(v->data_buffer);
        if ( v->ecus != null ) {
            for(int i = 0; i < v->ecus_len; i++) {
                vehicle_ecu_free(v->ecus[i]);
            }
            free(v->ecus);
            v->ecus = null;
        }
        v->ecus_len = 0;
        MEMORY_FREE_POINTER(v->country)
        MEMORY_FREE_POINTER(v->manufacturer)
        v->year = VEHICLE_YEAR_EMPTY;
        MEMORY_FREE_POINTER(v->engine)
        MEMORY_FREE_POINTER(v->internal.directory)
        if ( v->vin != null ) {
            buffer_free(v->vin);
            v->vin = null;
        }
        list_Buffer_free(v->internal.filter);
        ehh_free(v->internal.events.onECURegister);
        ehh_free(v->internal.events.onFilterChange);
        free(v);
    }
}

void vehicle_fill_global_data_buffer_from_ecus(Vehicle* v) {
    list_Buffer_empty(v->data_buffer);
    for ( int i = 0; i < v->ecus_len; i++) {
        ECU* ecu = v->ecus[i];
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            list_Buffer_append(v->data_buffer,buffer_copy(ecu->data_buffer->list[j]));
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
    printf("  data_buffer: %p\n", v->data_buffer);
    printf("}\n");
}

void vehicle_dump(Vehicle* v) {
    log_msg(LOG_DEBUG, "Vehicle dump");
    log_msg(LOG_DEBUG, "Global data");
    list_Buffer_dump(v->data_buffer);
    for(int i = 0; i < v->ecus_len; i++) {
        log_msg(LOG_DEBUG, "Dump of one ECU");
        ECU * ecu = v->ecus[i];
        log_msg(LOG_DEBUG, "Address");
        Buffer * address = ecu->address;
        buffer_dump(address);
        log_msg(LOG_DEBUG, "content");
        list_Buffer_dump(ecu->data_buffer);

        log_msg(LOG_DEBUG, "in current_data");
        list_Buffer_dump(ecu->obd_service.current_data);
        log_msg(LOG_DEBUG, "in freeze_frame_data");
        list_Buffer_dump(ecu->obd_service.freeze_frame_data);
        log_msg(LOG_DEBUG, "in tests_results");
        list_Buffer_dump(ecu->obd_service.tests_results);
        log_msg(LOG_DEBUG, "in tests_results_other");
        list_Buffer_dump(ecu->obd_service.tests_results_other);
        log_msg(LOG_DEBUG, "in control_operation");
        list_Buffer_dump(ecu->obd_service.control_operation);
        log_msg(LOG_DEBUG, "in pending_dtc");
        list_Buffer_dump(ecu->obd_service.pending_dtc);
        log_msg(LOG_DEBUG, "in none");
        list_Buffer_dump(ecu->obd_service.none);
        log_msg(LOG_DEBUG, "in current_dtc");
        list_Buffer_dump(ecu->obd_service.current_dtc);
        log_msg(LOG_DEBUG, "in clear_dtc");
        list_Buffer_dump(ecu->obd_service.clear_dtc);
        log_msg(LOG_DEBUG, "in request_vehicle_information");
        list_Buffer_dump(ecu->obd_service.request_vehicle_information);
        log_msg(LOG_DEBUG, "in permanent_dtc");
        list_Buffer_dump(ecu->obd_service.permanent_dtc);
    
    }
}

int Vehicle_cmp(Vehicle* e1, Vehicle* e2) {
    return e1 - e2;
}
AD_LIST_SRC(Vehicle)

void list_Vehicle_empty(list_Vehicle * list) {
    assert(list != null);
    for(int i = 0; i < list->size; i ++) {
        if ( list->list[i] != null ) {
            vehicle_free(list->list[i]);
            list->list[i] = null;
        }
    }
    free(list->list);
    list->list = null;
    list->size = 0;
}