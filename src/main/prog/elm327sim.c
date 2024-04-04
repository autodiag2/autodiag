#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "sim/sim.h"

void display_help() {
    printf("ELM327 simulator\n");
    printf(" -h         : display this help\n");
    printf(" -e hh      : add an ecu to the simulation with address hh\n");
    printf(" -p         : list protocols\n");
    printf(" -p h       : set protocol to h\n");
    printf(" -p Ah      : set protocol to automatic, h\n");
    printf(" -l         : list level of logging\n");
    printf(" -l level   : set level of logging\n");
}

void display_protocols() {
    printf("Supported protocols:\n");
    for(int p = 0x1; p <= 0xC; p += 1) {
        printf("%1x : %s\n", p, elm327_protocol_to_string(p));
    }
}
int main(int argc, char**argv) {
    log_set_from_env();

    ELM327emulation* sim = elm327_sim_new();
    ELM327_PROTO *proto = null;
    bool * proto_is_auto = null;
    
    int opt;
    while ((opt = getopt(argc, argv, "he:l:p:")) != -1) {
        switch (opt) {
            case 'h': {
                display_help();
                return 0;
            }
            case 'e': {
                byte ecu_address;
                if ( sscanf(optarg,"%02hhx", &ecu_address) == 1 ) {
                    ECUEmulation_list_append(sim->ecus,ecu_emulation_new(ecu_address));                    
                } else {
                    display_help();
                    return 1;
                }
            } break;
            case 'p': {
                int p;
                if ( sscanf(optarg,"A%1x", &p) == 1 ) {
                    proto = (ELM327_PROTO *)intdup(p);
                    proto_is_auto = intdup(true);
                } else if ( sscanf(optarg,"%1x", &p) == 1 ) {
                    proto = (ELM327_PROTO *)intdup(p);
                    proto_is_auto = intdup(false);
                } else {
                    display_help();
                    return 1;
                }
            } break;
            case 'l': {
                logger.current_level = log_level_from_str(optarg);
            } break;
            case '?': {
                switch ( optopt ) {
                    case 'p':
                        display_protocols();
                        break;
                    case 'l':
                        printf("log levels:\n");
                        printf("none\n");
                        printf("error\n");                        
                        printf("warning\n");                        
                        printf("info\n");                        
                        printf("debug\n");    
                        break; 
                    case 'e':
                        printf("example: -e E8\n");                   
                        break;
                }
                return 1;
            }
        }
    }
    pthread_t t = elm327_sim_loop_thread(sim);
    usleep(50e3);
    if ( sim->ptsname == null ) {
        printf("Simualtion not started\n");
        return 1;
    } else {
        printf("Simulation running on %s\n", sim->ptsname);
        if ( proto != null ) {
            sim->protocolRunning = *proto;
        }
        if ( proto_is_auto != null ) {
            sim->protocol_is_auto_running = *proto_is_auto;
        }
        pthread_join(t, NULL);
        return 0;
    }
}
