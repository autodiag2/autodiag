#ifndef __SIM_ELM327_SIM_GENERATORS_H
#define __SIM_ELM327_SIM_GENERATORS_H

#include "lib/lib.h"
#include "elm327.h"

void ecu_saej1979_sim_generator_random(char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin);

#endif