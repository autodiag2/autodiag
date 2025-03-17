#ifndef __SIM_NVM_H
#define __SIM_NVM_H

#include "lib/lib.h"
#include "com/serial/elm/elm327/elm327.h"
#include "elm327.h"

/**
 * Holds what type of init is performed.
 */
typedef enum {
    ELM327_SIM_INIT_TYPE_DEFAULTS,
    ELM327_SIM_INIT_TYPE_RESET,
    ELM327_SIM_INIT_TYPE_POWER_OFF
} ELM327_SIM_INIT_TYPE;

bool elm327_sim_non_volatile_memory_load(ELM327emulation * elm327, final ELM327_SIM_INIT_TYPE type);
bool elm327_sim_non_volatile_memory_store(ELM327emulation * elm327);
bool elm327_sim_non_volatile_wipe_out();

#endif