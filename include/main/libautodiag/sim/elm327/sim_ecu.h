#ifndef __SIM_ELM327_SIM_H
#define __SIM_ELM327_SIM_H

#include "libautodiag/sim/ecu/generator.h"
#include "libautodiag/com/uds/uds.h"

struct _SimELM327;

typedef struct SimECU {
    byte address;
    SimECUGenerator * generator;
    /**
     * Respond to use request (at commands, OBD, UDS)
     */
    char * (*sim_ecu_response)(struct SimECU * ecu, struct _SimELM327 * elm327, char * request, bool hasSpaces);
} SimECU;
AD_LIST_H(SimECU)
#define LIST_SIM_ECU(var) ((list_SimECU*)var)
void list_SimECU_empty(list_SimECU * list);

#define ELM327_CAN_28_BITS_DEFAULT_PRIO 0x18

SimECU* sim_ecu_emulation_new(byte address);
/**
 * Generate a header as string for use in the current emulation.
 * Header sent by tester to ECU.
 */
char * sim_ecu_generate_request_header_bin(struct _SimELM327* elm327,byte source_address, byte can28bits_prio, bool print_spaces);
/**
 * Generate the header for the response by ECU to the tester.
 */
Buffer* sim_ecu_generate_response_header_bin(struct _SimELM327* elm327,SimECU * ecu, byte can28bits_prio);

#include "elm327.h"

#endif
