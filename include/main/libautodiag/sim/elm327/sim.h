#ifndef __SIM_H
#define __SIM_H

#include "sim_generators.h"

struct _SimELM327;

typedef struct SimECU {
    byte address;
    SimECUGenerator * generator;
    /**
     * Respond to use request (at commands, OBD, UDS)
     */
    char * (*sim_ecu_response)(struct SimECU * ecu, struct _SimELM327 * elm327, char * request, bool hasSpaces);
} SimECU;
LIST_H(SimECU)
void SimECU_list_empty(SimECU_list * list);

#define ELM327_CAN_28_BITS_DEFAULT_PRIO 0x18

SimECU* sim_ecu_emulation_new(byte address);
/**
 * Generate a header as string for use in the current emulation.
 */
char * sim_ecu_generate_obd_header(struct _SimELM327* elm327,byte source_address, byte can28bits_prio, bool print_spaces);
Buffer* sim_ecu_generate_header_bin(struct _SimELM327* elm327,SimECU * ecu, byte can28bits_prio);

#include "elm327.h"

#endif
