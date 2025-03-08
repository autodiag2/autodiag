#ifndef __ELM327_SIM_H
#define __ELM327_SIM_H

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "lib/lib.h"
#include "com/serial/elm/elm327/elm327.h"

#define ELM327_SIM_DEFAULT_PROTO ELM327_PROTO_ISO_15765_4_CAN_1

typedef struct {
	char * eol;
	bool echo;
	ELM327_PROTO protocolRunning;
	bool protocol_is_auto_running;
	bool printing_of_spaces;
	bool printing_of_headers;
    char *dev_description;
    char *dev_identifier;
    char *port_name;
    double voltage;
    double voltageFactory;
    int baud_rate;
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
    pthread_t activity_monitor_thread;
    pthread_t loop_thread;
    
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
    	Buffer* programmable_parameters;
    	Buffer* programmable_parameters_states;    	
    } nvm;

    #ifdef OS_WINDOWS
        HANDLE pipe_handle;
    #elif defined OS_POSIX
        int fd;
    #else
    #   warning OS unsupported
    #endif
} _ELM327emulation;

/// TO BE DEPLACED TO SIM.h

typedef enum {
    ECUEmulationGeneratorTypeRandom,
    ECUEmulationGeneratorTypeCycle,
    ECUEmulationGeneratorTypeGui
} ECUEmulationGeneratorType;
typedef struct {
    ECUEmulationGeneratorType type;
    void *seed;
} ECUEmulationGenerator;

typedef struct {
    byte address;
    ECUEmulationGenerator generator;
} _ECUEmulation;

/**
 * Used to simulate an ECU on the bus
 */
typedef struct {
    _ECUEmulation;
    /**
     * Respond to an SAEJ1979 query
     */
    char * (*saej1979_sim_response)(_ECUEmulation * ecu, _ELM327emulation * elm327, char * obd_query_str, bool hasSpaces);
} ECUEmulation;
LIST_DEFINE_WITH_MEMBERS_AUTO(ECUEmulation)
///

#define ELM327_SIM_PPS_SZ 0x30
typedef struct {
    _ELM327emulation;
    ECUEmulation_list * ecus;
} ELM327emulation;

ELM327emulation* elm327_sim_new();
void elm327_sim_loop(ELM327emulation * elm327);
void elm327_sim_loop_start(ELM327emulation * elm327);
void elm327_sim_destroy(ELM327emulation * elm327);

#include "elm327_cli.h"
#include "nvm.h"

#endif
