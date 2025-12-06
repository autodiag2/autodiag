#ifndef __SIM_ECU_H
#define __SIM_ECU_H

#include "libautodiag/sim/ecu/generator.h"

typedef struct SimECU {
    byte address;
    SimECUGenerator * generator;
} SimECU;

AD_LIST_H(SimECU)
#define LIST_SIM_ECU(var) ((list_SimECU*)var)
void list_SimECU_empty(list_SimECU * list);

SimECU* sim_ecu_new(byte address);
Buffer * sim_ecu_response(SimECU * ecu, Buffer * binRequest);

#endif
