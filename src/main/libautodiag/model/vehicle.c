#include "libautodiag/model/vehicle.h"

int ad_object_ECU_cmp(ad_object_ECU * ecu1, ad_object_ECU * ecu2) {
    return ecu1 - ecu2;
}
AD_LIST_SRC(ad_object_ECU)

ad_object_ECU* vehicle_ecu_add_if_not_in(Vehicle* v, byte* address, int size) {
    for(int i = 0; i < v->ecus->size; i++) {
        ad_object_ECU* ecu = v->ecus->list[i];
        if ( size == ecu->address->size ) {
            if ( memcmp(ecu->address->buffer,address, size) == 0 ) {
                return ecu;
            }        
        }
    }
    log_msg(LOG_DEBUG, "New ECU added");
    return vehicle_ecu_add(v,address,size);
}

ad_object_ECU* vehicle_ecu_add(Vehicle* v, byte* address, int size) {
    final ad_object_ECU* ecu = ad_object_ECU_new();
    ad_buffer_append_bytes(ecu->address,address,size);
    ad_list_ad_object_ECU_append(v->ecus, ecu);
    return ecu;
}

ad_object_ECU* ad_object_ECU_new() {
    ad_object_ECU* ecu = (ad_object_ECU*)malloc(sizeof(ad_object_ECU));
    ecu->address = ad_buffer_new();
    ecu->name = null;
    ecu->manufacturer = null;
    ecu->model = null;
    ecu->data_buffer = ad_list_Buffer_new();
    ecu->obd_service.current_data = ad_list_Buffer_new();
    ecu->obd_service.freeze_frame_data = ad_list_Buffer_new();
    ecu->obd_service.tests_results = ad_list_Buffer_new();
    ecu->obd_service.tests_results_other = ad_list_Buffer_new();
    ecu->obd_service.control_operation = ad_list_Buffer_new();
    ecu->obd_service.pending_dtc = ad_list_Buffer_new();
    ecu->obd_service.none = ad_list_Buffer_new();
    ecu->obd_service.current_dtc = ad_list_Buffer_new();
    ecu->obd_service.clear_dtc = ad_list_Buffer_new();
    ecu->obd_service.request_vehicle_information = ad_list_Buffer_new();
    ecu->obd_service.permanent_dtc = ad_list_Buffer_new();
    return ecu;
}
ad_object_ECU* vehicle_search_ecu_by_address(Vehicle* v, Buffer* address) {
    for(int i = 0; i < v->ecus->size; i++) {
        if ( ad_buffer_cmp(v->ecus->list[i]->address, address) == 0 ) {
            return v->ecus->list[i];
        }
    }
    return null;
}
void vehicle_ecu_empty(ad_object_ECU* ecu) {
    ad_list_Buffer_empty(ecu->data_buffer);
    vehicle_ecu_empty_duplicated_info(ecu);
}
void vehicle_ecu_empty_duplicated_info(ad_object_ECU* ecu) {
    ad_list_Buffer_empty(ecu->obd_service.current_data);
    ad_list_Buffer_empty(ecu->obd_service.freeze_frame_data);
    ad_list_Buffer_empty(ecu->obd_service.tests_results);
    ad_list_Buffer_empty(ecu->obd_service.tests_results_other);
    ad_list_Buffer_empty(ecu->obd_service.control_operation);
    ad_list_Buffer_empty(ecu->obd_service.pending_dtc);
    ad_list_Buffer_empty(ecu->obd_service.none);
    ad_list_Buffer_empty(ecu->obd_service.current_dtc);
    ad_list_Buffer_empty(ecu->obd_service.clear_dtc);
    ad_list_Buffer_empty(ecu->obd_service.request_vehicle_information);
    ad_list_Buffer_empty(ecu->obd_service.permanent_dtc);
}
void ad_object_ECU_free(ad_object_ECU* ecu) {
    ad_list_Buffer_free(ecu->data_buffer);
    MEMORY_FREE_POINTER(ecu->address);
    MEMORY_FREE_POINTER(ecu->name);
    MEMORY_FREE_POINTER(ecu->manufacturer);
    MEMORY_FREE_POINTER(ecu->model);
}
void vehicle_ecu_debug(final ad_object_ECU *ecu) {
    assert(ecu != null);
    printf("ECU debug: {\n");
    printf("  address: %p\n", ecu->address);
    printf("  name: %s\n", ecu->name ? ecu->name : "null");
    printf("  mode: %s\n", ecu->model);
    printf("  manufacturer: %s\n", ecu->manufacturer);
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
    v->ecus = ad_list_ad_object_ECU_new();
    v->data_buffer = ad_list_Buffer_new();
    v->country = null;
    v->manufacturer = null;
    v->model = null;
    v->engine_manufacturer = null;
    v->year = VEHICLE_YEAR_EMPTY;
    v->engine = null;
    v->vin = null;
    v->internal.events.onECURegister = ehh_new();
    v->internal.events.onFilterChange = ehh_new();
    v->internal.filter = ad_list_Buffer_new();
    return v;
}

void vehicle_free(Vehicle * v) {
    if ( v != null ) {
        ad_list_Buffer_free(v->data_buffer);
        if ( v->ecus != null ) {
            for(int i = 0; i < v->ecus->size; i++) {
                ad_object_ECU_free(v->ecus->list[i]);
            }
            ad_list_ad_object_ECU_free(v->ecus);
        }
        MEMORY_FREE_POINTER(v->country)
        MEMORY_FREE_POINTER(v->manufacturer)
        v->year = VEHICLE_YEAR_EMPTY;
        MEMORY_FREE_POINTER(v->engine)
        MEMORY_FREE_POINTER(v->model);
        MEMORY_FREE_POINTER(v->engine_manufacturer);
        if ( v->vin != null ) {
            ad_buffer_free(v->vin);
            v->vin = null;
        }
        ad_list_Buffer_free(v->internal.filter);
        ehh_free(v->internal.events.onECURegister);
        ehh_free(v->internal.events.onFilterChange);
        free(v);
    }
}

void vehicle_fill_global_data_buffer_from_ecus(Vehicle* v) {
    ad_list_Buffer_empty(v->data_buffer);
    for ( int i = 0; i < v->ecus->size; i++) {
        ad_object_ECU* ecu = v->ecus->list[i];
        for(int j = 0; j < ecu->data_buffer->size; j++) {
            ad_list_Buffer_append(v->data_buffer,ad_buffer_copy(ecu->data_buffer->list[j]));
        }
    }
}
void vehicle_debug(Vehicle* v) {
    printf("Vehicle debug: {\n");
    printf("  ecus: %p\n", v->ecus);
    for(int i = 0; i < v->ecus->size; i++) {
        ad_object_ECU * ecu = v->ecus->list[i];
        vehicle_ecu_debug(ecu);
    }
    printf("  data_buffer: %p\n", v->data_buffer);
    printf("}\n");
}

void vehicle_dump(Vehicle* v) {
    assert(v != null);
    log_msg(LOG_DEBUG, "Vehicle dump");
    log_msg(LOG_DEBUG, "Global data");
    ad_list_Buffer_dump(v->data_buffer);
    for(int i = 0; i < v->ecus->size; i++) {
        log_msg(LOG_DEBUG, "Dump of one ECU");
        ad_object_ECU * ecu = v->ecus->list[i];
        log_msg(LOG_DEBUG, "Address");
        Buffer * address = ecu->address;
        ad_buffer_dump(address);
        log_msg(LOG_DEBUG, "content");
        ad_list_Buffer_dump(ecu->data_buffer);

        log_msg(LOG_DEBUG, "in current_data");
        ad_list_Buffer_dump(ecu->obd_service.current_data);
        log_msg(LOG_DEBUG, "in freeze_frame_data");
        ad_list_Buffer_dump(ecu->obd_service.freeze_frame_data);
        log_msg(LOG_DEBUG, "in tests_results");
        ad_list_Buffer_dump(ecu->obd_service.tests_results);
        log_msg(LOG_DEBUG, "in tests_results_other");
        ad_list_Buffer_dump(ecu->obd_service.tests_results_other);
        log_msg(LOG_DEBUG, "in control_operation");
        ad_list_Buffer_dump(ecu->obd_service.control_operation);
        log_msg(LOG_DEBUG, "in pending_dtc");
        ad_list_Buffer_dump(ecu->obd_service.pending_dtc);
        log_msg(LOG_DEBUG, "in none");
        ad_list_Buffer_dump(ecu->obd_service.none);
        log_msg(LOG_DEBUG, "in current_dtc");
        ad_list_Buffer_dump(ecu->obd_service.current_dtc);
        log_msg(LOG_DEBUG, "in clear_dtc");
        ad_list_Buffer_dump(ecu->obd_service.clear_dtc);
        log_msg(LOG_DEBUG, "in request_vehicle_information");
        ad_list_Buffer_dump(ecu->obd_service.request_vehicle_information);
        log_msg(LOG_DEBUG, "in permanent_dtc");
        ad_list_Buffer_dump(ecu->obd_service.permanent_dtc);
    
    }
}

int Vehicle_cmp(Vehicle* e1, Vehicle* e2) {
    return e1 - e2;
}
AD_LIST_SRC(Vehicle)

void ad_list_Vehicle_empty(ad_list_Vehicle * list) {
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