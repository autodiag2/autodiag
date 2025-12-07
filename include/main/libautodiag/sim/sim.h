#ifndef __SIM_H
#define __SIM_H

#include "libautodiag/sim/ecu/ecu.h"

typedef struct {
    /**
     * Holds the list of ECUs in this simulation
     */
    list_SimECU * ecus;
} Sim;

#define SIM(var) ((Sim*)var)

/**
 * Load the sim contained in the json context, any existing sim settings are dropped.
 * @param json_context filepath or string containing the json
 * @param sim sim into which to load the context (eg ELM327 sim)
 */
int sim_load_from_json(Sim * sim, char * json_context);

#include "libautodiag/sim/elm327/elm327.h"

#endif