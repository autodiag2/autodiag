#ifndef __SIM_SIM_ELM327_GENERATORS_H
#define __SIM_SIM_ELM327_GENERATORS_H

#include "libautodiag/lib.h"
#include "libautodiag/com/obd/obd.h"

typedef struct SimECUGenerator {
    void *context;
    char *type;
    Buffer * (*response)(struct SimECUGenerator * this, final Buffer *binRequest);
    void (*response_for_python)(struct SimECUGenerator * this, final Buffer *binRequest, final Buffer * binResponse);
} SimECUGenerator;

void sim_ecu_generator_fill_nrc(Buffer * binResponse, Buffer * binRequest, byte nrc);
void sim_ecu_generator_fill_success(Buffer * binResponse, Buffer * binRequest);
SimECUGenerator* sim_ecu_generator_new_cycle();
SimECUGenerator* sim_ecu_generator_new_random();
SimECUGenerator * sim_ecu_generator_new();
SimECUGenerator* sim_ecu_generator_new_citroen_c5_x7();
#define SIM_ECU_GENERATOR_RESPONSE_FUNC(f) ((Buffer* (*)(SimECUGenerator *, Buffer *))f)

#endif