#ifndef __SIM_ELM327_NETWORK_H
#define __SIM_ELM327_NETWORK_H

#include "libautodiag/lib.h"
#include "libautodiag/com/network.h"

#define ELM327_NETWORK_PORT 35000
bool sim_elm327_network_is_connected(void * implPtr);

#endif