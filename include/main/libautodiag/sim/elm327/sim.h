#ifndef __SIM_H
#define __SIM_H

#include "sim_generators.h"

struct _ELM327emulation;

typedef struct ECUEmulation {
    byte address;
    ECUEmulationGenerator * generator;
    /**
     * Respond to an SAEJ1979 query
     */
    char * (*saej1979_sim_response)(struct ECUEmulation * ecu, struct _ELM327emulation * elm327, char * obd_query_str, bool hasSpaces);
} ECUEmulation;
LIST_DEFINE_WITH_MEMBERS_AUTO(ECUEmulation)

#define ELM327_CAN_28_BITS_DEFAULT_PRIO 0x18

ECUEmulation* ecu_emulation_new(byte address);
/**
 * Generate a header as string for use in the current emulation.
 */
char * ecu_sim_generate_obd_header(struct _ELM327emulation* elm327,byte source_address, byte can28bits_prio, bool print_spaces);
Buffer* ecu_sim_generate_header_bin(struct _ELM327emulation* elm327,ECUEmulation * ecu, byte can28bits_prio);

#include "elm327.h"

#endif
