#ifndef __SIM_NVM_H
#define __SIM_NVM_H

#include "lib/lib.h"
#include "com/serial/elm/elm327/elm327.h"
#include "elm327.h"

#define ELM327_SIM_NON_VOLATILE_MEMORY_PATH "/tmp/sim_memory.txt"

bool elm327_sim_non_volatile_memory_load(ELM327emulation * elm327);
bool elm327_sim_non_volatile_memory_store(ELM327emulation * elm327);

#endif