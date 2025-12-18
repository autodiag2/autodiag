#ifndef __SIM_H
#define __SIM_H

#include "libautodiag/sim/ecu/ecu.h"

typedef struct {
    /**
     * Holds the list of ECUs in this simulation
     */
    list_SimECU * ecus;
} Sim;

#define SIM(var) ((Sim*)var)

/**
 * Load the sim contained in the json context, any existing sim settings are dropped.
 * @param json_context filepath or string containing the json
 * @param sim sim into which to load the context (eg ELM327 sim)
 */
int sim_load_from_json(Sim * sim, char * json_context);
int sim_write(void *implPtr, int timeout_ms, byte * data, unsigned int data_len);
int sim_read(void *implPtr, int timeout_ms, Buffer * readed);
/**
 * Namedpipes for windows for instance,
 * if we don't make a wait cause the next read to read it-self not the reply of client.
 */
void sim_prevent_read_himself(void * implPtr);

#include "libautodiag/sim/elm327/elm327.h"

#endif