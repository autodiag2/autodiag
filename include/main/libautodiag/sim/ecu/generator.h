#ifndef __SIM_ECU_GENERATOR_H
#define __SIM_ECU_GENERATOR_H

#include "libautodiag/lib.h"
#include "libautodiag/com/obd/obd.h"
#include "libautodiag/com/uds/uds.h"

typedef struct SimECUGenerator {
    /**
     * Context to be set by the caller for the generator to work.
     */
    void *context;
    /**
     * Plain text type of the generator (e.g., "Citroen C5 X7", "cycle", "random", etc.)
     */
    char *type;
    /**
     * Holds the state of the generator (like DTCs, session type, etc.)
     * it is not expected to be modified by external parts using the generator.
     */
    void *state;
    /**
     * Generates a response for the given request (OBD/UDS or any data protocol over for example CAN messages)
     * @param this Pointer to the SimECUGenerator instance
     * @param binRequest Pointer to the binary request buffer
     * @return Pointer to the binary response buffer
     */
    Buffer * (*response)(struct SimECUGenerator * this, final Buffer *binRequest);
    /**
     * Generates a response for the given request (OBD/UDS or any data protocol over for example CAN messages)
     * @param this Pointer to the SimECUGenerator instance
     * @param binRequest Pointer to the binary request buffer
     * @param binResponse Pointer to the preallocated response buffer
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