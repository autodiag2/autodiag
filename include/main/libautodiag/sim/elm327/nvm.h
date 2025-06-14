#ifndef __SIM_NVM_H
#define __SIM_NVM_H

#include "libautodiag/lib.h"
#include "libautodiag/com/serial/elm/elm327/elm327.h"
#include "elm327.h"

/**
 * Holds what type of init is performed.
 */
typedef enum {
    ELM327_SIM_INIT_TYPE_DEFAULTS = 4,
    ELM327_SIM_INIT_TYPE_RESET = 2,
    ELM327_SIM_INIT_TYPE_POWER_OFF = 1,
    ELM327_SIM_INIT_TYPE_IMMEDIATE = ELM327_SIM_INIT_TYPE_DEFAULTS | ELM327_SIM_INIT_TYPE_RESET | ELM327_SIM_INIT_TYPE_POWER_OFF
} ELM327_SIM_INIT_TYPE;

bool elm327_sim_non_volatile_memory_load(SimELM327 * elm327, final int load_mask);
bool elm327_sim_non_volatile_memory_store(SimELM327 * elm327);
bool elm327_sim_non_volatile_wipe_out();

#endif