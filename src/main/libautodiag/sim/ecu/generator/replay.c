#include "libautodiag/sim/ecu/generator.h"

static Buffer * response(SimECUGenerator *generator, final Buffer *binRequest) {
    if ( generator->context == null ) {
        log_msg(LOG_ERROR, "Need a filepath for this generator");
        assert(generator->context != null);
    }
}

SimECUGenerator* sim_ecu_generator_new_replay() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->response = SIM_ECU_GENERATOR_RESPONSE(response);
    generator->type = strdup("replay");
    return generator;
}