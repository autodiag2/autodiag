#ifndef __SIM_ELM327_SIM_GENERATORS_H
#define __SIM_ELM327_SIM_GENERATORS_H

#include "lib/lib.h"
#include "elm327.h"

/**
 * Go to the next cycle.
 */
void ecu_saej1979_sim_generator_cycle_iterate();
/**
 * Cyclic iteration over values.
 */
void ecu_saej1979_sim_generator_cycle(char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin);
/**
 * Random data.
 */
void ecu_saej1979_sim_generator_random(char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin);

#endif