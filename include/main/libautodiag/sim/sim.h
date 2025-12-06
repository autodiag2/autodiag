#ifndef __SIM_H
#define __SIM_H

#include "libautodiag/sim/elm327/elm327.h"
#include "libautodiag/sim/ecu/generator.h"

int sim_load_from_json(SimELM327 * elm327, char * json_context);

#endif