#ifndef __SIM_ECU_GENERATOR_H
#define __SIM_ECU_GENERATOR_H

#include "libautodiag/lib.h"
#include "libautodiag/com/obd/obd.h"
#include "libautodiag/com/uds/uds.h"

typedef struct SimECUGenerator {
    void *context;
    char *type;
    /**
     * Generates a response for the given request (OBD/UDS or any data protocol over for example CAN stack)
     * @param this Pointer to the SimECUGenerator instance
     * @param binRequest Pointer to the binary request buffer
     * @return Pointer to the binary response buffer
     */
    Buffer * (*response)(struct SimECUGenerator * this, final Buffer *binRequest);
    /**
     * Same as 'response' but designed to be called from Python code
     */
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