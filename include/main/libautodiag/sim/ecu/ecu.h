#ifndef __AD_SIM_ECU_H
#define __AD_SIM_ECU_H

#include "libautodiag/sim/ecu/generator.h"
#include "libautodiag/dependencies/json.h"

typedef byte ECU_address;
#define ECU_address_assign(ecu_address, address) ecu_address = address
static inline double ECU_address_to_double(ECU_address address) {
    return (double)(int8_t)address;
}

static inline ECU_address ECU_address_from_double(double value) {
    return (ECU_address)(int8_t)value;
}

typedef struct SimECU {
    ECU_address address;
    SimECUGenerator * generator;
} SimECU;

AD_LIST_H(SimECU)
#define LIST_SIM_ECU(var) ((ad_list_SimECU*)var)
void ad_list_SimECU_empty(ad_list_SimECU * list);

SimECU* sim_ecu_new(ECU_address address);
Buffer * sim_ecu_response(SimECU * ecu, Buffer * binRequest);
SimECU * ad_list_SimECU_search_by_address(ad_list_SimECU * list, ECU_address address);

#define SIM_ECU_DEFAULT_ADDRESS 0xE8
#define SIM_ECU_SCHEMA "autodiag/sim/ecu"
#define SIM_ECU_SCHEMA_VERSION 1.0

cJSON * ad_object_SimECU_to_json(SimECU * ecu);
bool ad_object_SimECU_from_json(SimECU * ecu, cJSON * json);

typedef SimECUGenerator *(*SimECUGeneratorNewFunc)();
/**
 * Use this to add a generator type.
 */
void ad_object_SimECU_register_generator(
    const char *name,
    SimECUGeneratorNewFunc generator_new_func);

#endif
