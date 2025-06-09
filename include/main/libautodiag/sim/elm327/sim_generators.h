#ifndef __SIM_ELM327_SIM_GENERATORS_H
#define __SIM_ELM327_SIM_GENERATORS_H

#include "libautodiag/lib.h"
#include "elm327.h"

/**
 * Cyclic iteration over values.
 */
void ecu_saej1979_sim_generator_cycle(ECUEmulationGenerator *generator, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin);
/**
 * Random data.
 */
void ecu_saej1979_sim_generator_random(ECUEmulationGenerator *generator, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin);
/**
 * Set the data from a gui.
 */
void ecu_saej1979_sim_generator_gui(ECUEmulationGenerator *generator, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin);

#endif