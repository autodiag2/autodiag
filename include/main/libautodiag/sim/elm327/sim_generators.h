#ifndef __SIM_ELM327_SIM_GENERATORS_H
#define __SIM_ELM327_SIM_GENERATORS_H

#include "libautodiag/lib.h"

typedef struct ECUEmulationGenerator {
    void *context;
    char *type;
    void * (*obd_sim_response)(struct ECUEmulationGenerator * this, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin);
} ECUEmulationGenerator;

ECUEmulationGenerator* sim_ecu_generator_new_cycle();
ECUEmulationGenerator* sim_ecu_generator_new_random();
ECUEmulationGenerator * sim_ecu_generator_new();
#define SIM_ECU_GENERATOR_RESPONSE_FUNC(f) ((void *(*)(ECUEmulationGenerator *, char **, Buffer *, Buffer *))f)

#endif