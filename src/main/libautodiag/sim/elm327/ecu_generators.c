#include "libautodiag/sim/elm327/sim_generators.h"

SimECUGenerator * sim_ecu_generator_new() {
    SimECUGenerator * generator = (SimECUGenerator*)malloc(sizeof(SimECUGenerator));
    generator->context = null;
    generator->sim_ecu_generator_response = null;
    generator->type = null;
    return generator;
}