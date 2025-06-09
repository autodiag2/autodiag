#ifndef __SIM_ELM327_SIM_GENERATORS_H
#define __SIM_ELM327_SIM_GENERATORS_H

#include "libautodiag/lib.h"
#include "elm327.h"

ECUEmulationGenerator* sim_ecu_generator_new_cycle();
ECUEmulationGenerator* sim_ecu_generator_new_random();
ECUEmulationGenerator * sim_ecu_generator_new();

#endif