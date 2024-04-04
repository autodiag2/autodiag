#ifndef __SIM_H
#define __SIM_H

#include "elm327.h"

#define ELM327_CAN_28_BITS_DEFAULT_PRIO 0x18

ECUEmulation* ecu_emulation_new(byte address);
/**
 * Generate a header as string for use in the current emulation.
 */
char * ecu_sim_generate_header(ELM327emulation* elm327,byte source_address, byte can28bits_prio);
Buffer* ecu_sim_generate_header_bin(ELM327emulation* elm327,ECUEmulation * ecu, byte can28bits_prio);

#endif
