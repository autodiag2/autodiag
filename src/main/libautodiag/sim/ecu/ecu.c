#include "libautodiag/sim/ecu/ecu.h"
#include "libautodiag/eventHandlerHolder.h"

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
ad_object_Ptr * ad_simECU_conversation_get_by_address(SimECU * ecu, Buffer * address) {
    for (int i = 0; i < ecu->conversations->size; i++) {
        Buffer * conv_address = ecu->conversations->keys[i]->value;
        if ( ad_buffer_cmp(conv_address, address) == 0 ) {
            return ecu->conversations->values[i];
        }
    }
    return null;
}

ad_object_Ptr * ad_simECU_conversation_set_by_address(SimECU * ecu, Buffer * address, ad_object_Ptr * value) {
    for (int i = 0; i < ecu->conversations->size; i++) {
        Buffer * conv_address = ecu->conversations->keys[i]->value;
        if ( ad_buffer_cmp(conv_address, address) == 0 ) {
            ecu->conversations->values[i] = value;
            return value;
        }
    }
    ad_object_hashmap_Ptr_Ptr_set(ecu->conversations, ad_object_Ptr_new_from(address), value);
    return ad_simECU_conversation_get_by_address(ecu, address);
}
SimECU * ad_list_SimECU_search_by_address(ad_list_SimECU * list, ECU_address address) {
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
    if ( 0 < binResponse->size ) {
        byte service = binResponse->buffer[0];
        switch(service) {
            case OBD_SERVICE_SHOW_CURRENT_DATA | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE:
            case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA | OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE: {
                if ( 1 < binResponse->size ) {
                    byte pid = binResponse->buffer[1];
                    ad_object_vehicle_signal * signal = ad_signal_get_from_saej1979_pid(pid);
                    if ( signal != null ) {
                        char * error = null;
                        Buffer * signal_data = ad_buffer_slice(binResponse, signal->rv_offset_bytes(binResponse, signal->input_formula), binResponse->size - signal->rv_offset_bytes(binResponse, signal->input_formula));
                        double value = ad_expr_reduce_buffer(signal_data, signal->rv_formula, &error);
                        ad_buffer_free(signal_data);
                        if ( NAN == value ) {
                            log_debug("error while reducing");
                        } else {
                            ehh_trigger(ecu->generator->onSignalReceived, (void (*)(SimECUGenerator*, ad_object_vehicle_signal *, double)), ecu->generator, signal, value);
                        }
                    }
                }
            } break;
        }
    }
    return binResponse;
}

SimECU* sim_ecu_new(ECU_address address) {
    final SimECU* emu = (SimECU*)malloc(sizeof(SimECU));
    emu->address = address;
    emu->generator = sim_ecu_generator_new_random();
    emu->conversations = ad_object_hashmap_Ptr_Ptr_new();
    return emu;
}

static ad_object_hashmap_string_Ptr *generators = NULL;

static void ensureGeneratorsTable();

static SimECUGenerator *generator_new(const char *name)
{
    ensureGeneratorsTable();

    ad_object_Ptr *ptr = ad_object_hashmap_string_Ptr_get(
        generators,
        ad_object_string_new_from((char*)name)
    );

    if (ptr == null or ptr->value == null) {
        return null;
    }

    SimECUGeneratorNewFunc func = (SimECUGeneratorNewFunc)ptr->value;
    return func();
}

void ad_object_SimECU_register_generator(
    const char *name,
    SimECUGeneratorNewFunc generator_new_func)
{
    ensureGeneratorsTable();

    ad_object_hashmap_string_Ptr_set(
        generators,
        ad_object_string_new_from((char*)name),
        ad_object_Ptr_new_from((void *)generator_new_func)
    );
}

static void ensureGeneratorsTable()
{
    if (generators != null) {
        return;
    }

    generators = ad_object_hashmap_string_Ptr_new();

    ad_object_SimECU_register_generator("random", sim_ecu_generator_new_random);
    ad_object_SimECU_register_generator("cycle", sim_ecu_generator_new_cycle);
    ad_object_SimECU_register_generator("citroen_c5_x7", sim_ecu_generator_new_citroen_c5_x7);
    ad_object_SimECU_register_generator("replay", sim_ecu_generator_new_replay);
}

cJSON * ad_object_SimECU_to_json(SimECU * ecu) {
    assert(ecu != null);
    cJSON * json = cJSON_CreateObject();
    if ( ecu->generator == null ) {
        log_err("generator not attached");
        return json;
    }
    char * schema = gprintf("%s/%s", SIM_ECU_SCHEMA, ecu->generator->type);
    cJSON_AddStringToObject(json, "schema", schema);
    free(schema);
    cJSON_AddNumberToObject(json, "version", SIM_ECU_SCHEMA_VERSION);
    cJSON * content = cJSON_AddObjectToObject(json, "content");
    cJSON_AddStringToObject(content, "displayName", "");
    cJSON_AddNumberToObject(content, "address", ECU_address_to_double(ecu->address));

    cJSON *generator_json =
        ecu->generator->to_json == NULL
            ? cJSON_CreateObject()
            : ecu->generator->to_json(ecu->generator);

    while (generator_json->child != NULL) {
        cJSON *item = generator_json->child;
        cJSON_DetachItemViaPointer(generator_json, item);
        cJSON_AddItemToObject(content, item->string, item);
    }

    cJSON_Delete(generator_json);
       
    return json;
}
bool ad_object_SimECU_from_json(SimECU * ecu, cJSON * json) {
    assert(ecu != null);
    assert(json != null);
    char * schema = cJSON_GetStringItem(json, "schema", "");
    if ( strncmp(schema, SIM_ECU_SCHEMA, strlen(SIM_ECU_SCHEMA)) != 0 ) {
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
    double address = cJSON_GetNumberItem(content, "address");
    if ( address == NAN ) {
        address = SIM_ECU_DEFAULT_ADDRESS;
    }
    ECU_address_assign(ecu->address, ECU_address_from_double(address));
    char * displayName = cJSON_GetStringItem(content, "displayName", "");
    char * type = schema + strlen(SIM_ECU_SCHEMA);
    if ( *type == '/' ) {
        type ++;
    }

    if ( ecu->generator == null || strcmp(type, ecu->generator->type) != 0 ) {
        SimECUGenerator * gen = generator_new(type);
        if ( gen == null ) {
            log_msg(LOG_ERROR, "cannot instanciate gen type : %s", type);
            return false;
        }
        ecu->generator = gen;
    }
    if ( ecu->generator->from_json == null ) {
        log_msg(LOG_WARNING, "cannot load the state from json function not defined");
        return false;
    }
    return ecu->generator->from_json(ecu->generator, content);
}