#ifndef __SIM_SIM_ELM327_GENERATORS_H
#define __SIM_SIM_ELM327_GENERATORS_H

#include "libautodiag/lib.h"

typedef struct SimECUGenerator {
    void *context;
    char *type;
    void * (*obd_sim_response)(struct SimECUGenerator * this, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin);
} SimECUGenerator;

SimECUGenerator* sim_ecu_generator_new_cycle();
SimECUGenerator* sim_ecu_generator_new_random();
SimECUGenerator * sim_ecu_generator_new();
#define SIM_ECU_GENERATOR_RESPONSE_FUNC(f) ((void *(*)(SimECUGenerator *, char **, Buffer *, Buffer *))f)

#endif