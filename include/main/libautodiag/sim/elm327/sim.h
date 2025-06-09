#ifndef __SIM_H
#define __SIM_H

#include "elm327.h"
#include "sim_generators.h"
#define ELM327_CAN_28_BITS_DEFAULT_PRIO 0x18

ECUEmulation* ecu_emulation_new(byte address);
/**
 * Generate a header as string for use in the current emulation.
 */
char * ecu_sim_generate_obd_header(ELM327emulation* elm327,byte source_address, byte can28bits_prio, bool print_spaces);
Buffer* ecu_sim_generate_header_bin(ELM327emulation* elm327,ECUEmulation * ecu, byte can28bits_prio);
ECUEmulationGeneratorType ecu_sim_generator_from_string(final char *generator);

#endif
