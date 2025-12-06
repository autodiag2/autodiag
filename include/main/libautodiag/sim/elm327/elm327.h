#ifndef __SIM_ELM327_H
#define __SIM_ELM327_H

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "libautodiag/lib.h"
#include "libautodiag/com/serial/elm/elm327/elm327.h"
#include "libautodiag/sim/elm327/sim_ecu.h"
#include "libautodiag/sim/sim.h"

#define SIM_ELM327_DEFAULT_PROTO ELM327_PROTO_ISO_15765_4_CAN_1

typedef struct {
    pthread_t * activity_monitor_thread;
    pthread_t * loop_thread;
    bool loop_ready;
    int timeout_ms;
    #ifdef OS_WINDOWS
        HANDLE pipe_handle;
    #elif defined OS_POSIX
        int fd;
    #else
    #   warning OS unsupported
    #endif
} SimELM327Implementation;

typedef struct _SimELM327 {
    Sim;
    SimELM327Implementation* implementation;
	char * eol;
	bool echo;
	ELM327_PROTO protocolRunning;
	bool protocol_is_auto_running;
	bool printing_of_spaces;
	bool printing_of_headers;
    char *dev_description;
    char *dev_identifier;
    char *device_location;
    double voltage;
    double voltageFactory;
    /**
     * in bauds/bps
     */
    int baud_rate;
    int baud_rate_timeout_msec;
    /**
     * Unused by J1939 (0xA to 0xC)
     */
    byte * receive_address;
    /**
     * Set a custom header when requesting the vehicle
     * big endian
     */
    Buffer *custom_header;
    byte activity_monitor_count;
    int activity_monitor_timeout;
    
    /**
     * Time in ms to wait for a vehicle response
     */
    int vehicle_response_timeout;
    /**
     * Simulate adaptive timeout feature
     */
    bool vehicle_response_timeout_adaptive;
    Buffer *obd_buffer;
    /**
     * Turn printing of reponses by the adaptater on or off
     */
    bool responses;
    
    /**
     * Holds default values for PPs
     */
    Buffer *programmable_parameters_defaults;
    /**
     * Holds the type of load required for the prog parameter to be effective.
     */
    Buffer* programmable_parameters_pending_load_type;

    struct {
        bool auto_format;
        Buffer * mask;
        Buffer * filter;
        byte priority_29bits;
        bool extended_addressing;
        byte extended_addressing_target_address;
        int timeout_multiplier;
        bool display_dlc;
    } can;

    /**
     * Is non volatile memory enabled
     */
    bool isMemoryEnabled;
    byte testerAddress;
    
    struct {
        /**
         * One byte of non volatile memory
         */
        byte user_memory;        
    	ELM327_PROTO protocol;
    	bool protocol_is_auto;    	
        /**
         * Holds the states of the programmable parameters
         * already written to the nvm and loaded
         */
    	Buffer* programmable_parameters;
        /**
         * Holds the states of the programmable parameters
         * already written to the nvm but not loaded yet
         */
        Buffer* programmable_parameters_pending;
        /**
         * Holds the states of the programmable parameters.
         * OFF or ON.
         * A PP may be ON state but need atd to be effective.
         */
    	Buffer* programmable_parameters_states;    	
    } nvm;

} SimELM327;

#define SIM_ELM327_PPS_SZ 0x30

SimELM327* sim_elm327_new();
void sim_elm327_loop(SimELM327 * elm327);
void sim_elm327_loop_as_daemon(SimELM327 * elm327);
/**
 * Use this to wait for the daemon to be ready to receive some data.
 */
bool sim_elm327_loop_daemon_wait_ready(SimELM327 * elm327);
void sim_elm327_destroy(SimELM327 * elm327);
void sim_elm327_debug(final SimELM327 * elm327);

#include "nvm.h"

#endif
