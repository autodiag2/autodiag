#ifndef __SIM_NVM_H
#define __SIM_NVM_H

#include "lib/lib.h"
#include "com/serial/elm/elm327/elm327.h"
#include "elm327.h"

/**
 * Holds what type of init is performed.
 */
typedef enum {
    ELM327_SIM_INIT_TYPE_IMMEDIATE = 8,
    ELM327_SIM_INIT_TYPE_DEFAULTS = 4,
    ELM327_SIM_INIT_TYPE_RESET = 2,
    ELM327_SIM_INIT_TYPE_POWER_OFF = 1
} ELM327_SIM_INIT_TYPE;

bool elm327_sim_non_volatile_memory_load(ELM327emulation * elm327, final int load_mask);
bool elm327_sim_non_volatile_memory_store(ELM327emulation * elm327);
bool elm327_sim_non_volatile_wipe_out();

#endif