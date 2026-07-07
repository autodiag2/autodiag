#ifndef __AD_SIM_ECU_H
#define __AD_SIM_ECU_H

#include "libautodiag/sim/ecu/generator.h"
#include "libautodiag/dependencies/json.h"

typedef byte ECU_address;
#define ECU_address_assign(ecu_address, address) ecu_address = address

typedef struct SimECU {
    ECU_address address;
    SimECUGenerator * generator;
} SimECU;

AD_LIST_H(SimECU)
#define LIST_SIM_ECU(var) ((ad_list_SimECU*)var)
void ad_list_SimECU_empty(ad_list_SimECU * list);

SimECU* sim_ecu_new(byte address);
Buffer * sim_ecu_response(SimECU * ecu, Buffer * binRequest);
SimECU * ad_list_SimECU_search_by_address(ad_list_SimECU * list, byte address);

#define SIM_ECU_DEFAULT_ADDRESS 0xE8
#define SIM_ECU_SCHEMA "autodiag/sim/ecu"
#define SIM_ECU_SCHEMA_VERSION 1.0

cJSON * ad_object_SimECU_to_json(SimECU * ecu);
bool ad_object_SimECU_from_json(SimECU * ecu, cJSON * json);

#endif
