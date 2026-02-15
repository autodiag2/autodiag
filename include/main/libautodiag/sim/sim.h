#ifndef __SIM_H
#define __SIM_H

#include "libautodiag/sim/ecu/ecu.h"
#include "libautodiag/compile_target.h"
#include "libautodiag/sim/network.h"

typedef struct {
    #ifdef OS_POSIX
        int handle;
    #endif
    #ifdef OS_WINDOWS
        int server_fd;
        sock_t client_socket;
    #elif defined OS_POSIX
        int server_fd;
    #else
    #   warning OS unsupported
    #endif
} SimImplementation;

typedef struct {
    /**
     * Holds the list of ECUs in this simulation
     */
    list_SimECU * ecus;
    /**
     * elm327, doip
     */
    char * type;
    char * device_location;
    SimImplementation * implementation;
} Sim;

#define SIM(var) ((Sim*)var)

/**
 * Load the sim contained in the json context, any existing sim settings are dropped.
 * @param json_context filepath or string containing the json
 * @param sim sim into which to load the context (eg ELM327 sim)
 */
int sim_load_from_json(Sim * sim, char * json_context);
int sim_write(Sim * sim, int timeout_ms, byte * data, unsigned int data_len);
int sim_read(Sim * sim, int timeout_ms, Buffer * readed);
/**
 * Namedpipes for windows for instance,
 * if we don't make a wait cause the next read to read it-self not the reply of client.
 */
void sim_prevent_read_himself(Sim *sim);
void sim_init_with_defaults(Sim *sim);
/**
 * Wait for a flag to be ready
 */
bool sim_loop_daemon_wait_ready(bool * var);

#include "libautodiag/sim/elm327/elm327.h"
#include "libautodiag/sim/doip/doip.h"

#endif