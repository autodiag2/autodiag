#ifndef __SIM_H
#define __SIM_H

#include "libautodiag/sim/ecu/generator.h"

struct list_SimECU;

typedef struct {
    /**
     * Holds the list of ECUs in this simulation
     */
    struct list_SimECU * ecus;
} Sim;

#define SIM(var) ((Sim*)var)

int sim_load_from_json(Sim * sim, char * json_context);

#include "libautodiag/sim/elm327/elm327.h"

#endif