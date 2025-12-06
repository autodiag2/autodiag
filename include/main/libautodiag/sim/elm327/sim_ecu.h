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
    Buffer * (*sim_ecu_response)(struct _SimELM327 * elm327, struct SimECU * ecu, Buffer * binRequest);
} SimECU;
AD_LIST_H(SimECU)
#define LIST_SIM_ECU(var) ((list_SimECU*)var)
void list_SimECU_empty(list_SimECU * list);

#define ELM327_CAN_28_BITS_DEFAULT_PRIO 0x18

SimECU* sim_ecu_emulation_new(byte address);

#include "elm327.h"

#endif
