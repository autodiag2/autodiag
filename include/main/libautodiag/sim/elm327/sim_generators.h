#ifndef __SIM_SIM_ELM327_GENERATORS_H
#define __SIM_SIM_ELM327_GENERATORS_H

#include "libautodiag/lib.h"

typedef struct SimECUGenerator {
    void *context;
    char *type;
    void * (*response)(struct SimECUGenerator * this, char ** response, final Buffer *binResponse, final Buffer *binRequest);
} SimECUGenerator;

SimECUGenerator* sim_ecu_generator_new_cycle();
SimECUGenerator* sim_ecu_generator_new_random();
SimECUGenerator * sim_ecu_generator_new();
SimECUGenerator* sim_ecu_generator_new_citroen_c5_x7();
#define SIM_ECU_GENERATOR_RESPONSE_FUNC(f) ((void *(*)(SimECUGenerator *, char **, Buffer *, Buffer *))f)

#endif