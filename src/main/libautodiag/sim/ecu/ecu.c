#include "libautodiag/sim/ecu/ecu.h"

int SimECU_cmp(SimECU* e1, SimECU* e2) {
    return e1 - e2;
}
AD_LIST_SRC(SimECU)

void ad_list_SimECU_empty(ad_list_SimECU * list) {
    while(0 < list->size) {
        log_msg(LOG_DEBUG, "should free ecu here");
        ad_list_SimECU_remove_at(list, 0);
    }
}
SimECU * ad_list_SimECU_search_by_address(ad_list_SimECU * list, byte address) {
    for(int i = 0; i < list->size; i++) {
        if ( list->list[i]->address == address ) {
            return list->list[i];
        }
    }
    return null;
}
/**
 * Respond to use request (at commands, OBD, UDS)
 */
Buffer * sim_ecu_response(SimECU * ecu, Buffer * binRequest) {
    final Buffer* binResponse;
    if ( ecu->generator->response_for_python != null ) {
        binResponse = ad_buffer_new();
        ecu->generator->response_for_python(ecu->generator, binRequest, binResponse);
    } else {
        binResponse = ecu->generator->response(ecu->generator, binRequest);
    }
    return binResponse;
}

SimECU* sim_ecu_new(byte address) {
    final SimECU* emu = (SimECU*)malloc(sizeof(SimECU));
    emu->address = address;
    emu->generator = sim_ecu_generator_new_random();
    return emu;
}

cJSON * ad_object_SimECU_to_json(SimECU * ecu) {
    return null;
}
bool ad_object_SimECU_from_json(SimECU * ecu, cJSON * json) {
    char * schema = cJSON_GetStringItem(json, "schema", "");
    if ( strcmp(schema, SIM_ECU_SCHEMA) != 0 ) {
        log_msg(LOG_ERROR, "schema invalid : %s", schema);
        return false;
    }
    double version = cJSON_GetNumberItem(json, "version");
    if ( version != SIM_ECU_SCHEMA_VERSION ) {
        log_msg(LOG_ERROR, "invalid version");
        return false;
    }
    cJSON * content = cJSON_GetObjectItem(json, "content");
    if ( content == null ) {
        log_msg(LOG_ERROR, "no content");
        return false;
    }
    double address = cJSON_GetNumberItem(json, "address");
    if ( address == NAN ) {
        address = SIM_ECU_DEFAULT_ADDRESS;
    }
    ECU_address_assign(ecu->address, (ECU_address)address);
    char * displayName = cJSON_GetStringItem(json, "displayName", "");
    if ( ecu->generator->from_json == null ) {
        log_msg(LOG_WARNING, "no from json attached");
        return false;
    }
    return ecu->generator->from_json(ecu->generator, content);
}