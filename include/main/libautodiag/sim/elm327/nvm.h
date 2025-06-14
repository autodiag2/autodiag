#ifndef __SIM_NVM_H
#define __SIM_NVM_H

#include "libautodiag/lib.h"
#include "libautodiag/com/serial/elm/elm327/elm327.h"
#include "elm327.h"

/**
 * Holds what type of init is performed.
 */
typedef enum {
    SIM_ELM327_INIT_TYPE_DEFAULTS = 4,
    SIM_ELM327_INIT_TYPE_RESET = 2,
    SIM_ELM327_INIT_TYPE_POWER_OFF = 1,
    SIM_ELM327_INIT_TYPE_IMMEDIATE = SIM_ELM327_INIT_TYPE_DEFAULTS | SIM_ELM327_INIT_TYPE_RESET | SIM_ELM327_INIT_TYPE_POWER_OFF
} SIM_ELM327_INIT_TYPE;

bool sim_elm327_non_volatile_memory_load(SimELM327 * elm327, final int load_mask);
bool sim_elm327_non_volatile_memory_store(SimELM327 * elm327);
bool sim_elm327_non_volatile_wipe_out();

#endif