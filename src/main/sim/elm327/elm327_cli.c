#include "sim/elm327/elm327_cli.h"

PRINT_MODULAR(elm327_sim_cli_help,
    "\n"
    "ELM327 simulator\n"
    "\n"
    " -h         : display this help\n"
    " -e hh      : add an ecu to the simulation with address hh\n"
    " -p         : list protocols\n"
    " -p h       : set protocol to h\n"
    " -p Ah      : set protocol to automatic, h\n"
    " -l         : list level of logging\n"
    " -l level   : set level of logging\n"
)


void elm327_sim_cli_display_protocols() {
    printf("Supported protocols:\n");
    for(int p = 0x1; p <= 0xC; p += 1) {
        printf("%1x : %s\n", p, elm327_protocol_to_string(p));
    }
}

void elm327_sim_cli_display_help() {
    elm327_sim_cli_help("");
}

int elm327_sim_cli_main(int argc, char **argv) {
    ELM327emulation* sim = elm327_sim_new();
    ELM327_PROTO *proto = null;
    bool * proto_is_auto = null;
    
    int opt;
    optind = 1;
    while ((opt = getopt(argc, argv, "he:l:p:")) != -1) {
        switch (opt) {
            case 'h': {
                elm327_sim_cli_display_help();
                return 0;
            }
            case 'e': {
                byte ecu_address;
                if ( sscanf(optarg,"%02hhx", &ecu_address) == 1 ) {
                    ECUEmulation_list_append(sim->ecus,ecu_emulation_new(ecu_address));                    
                } else {
                    elm327_sim_cli_display_help();
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
                    elm327_sim_cli_display_help();
                    return 1;
                }
            } break;
            case 'l': {
                logger.current_level = log_level_from_str(optarg);
            } break;
            case '?': {
                switch ( optopt ) {
                    case 'p':
                        elm327_sim_cli_display_protocols();
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
    elm327_sim_loop_start(sim);
    usleep(50e3);
    if ( sim->port_name == null ) {
        log_msg(LOG_INFO, "Simulation not started");
        return 1;
    } else {
        printf("Simulation running on %s\n", sim->port_name);
        if ( proto != null ) {
            sim->protocolRunning = *proto;
        }
        if ( proto_is_auto != null ) {
            sim->protocol_is_auto_running = *proto_is_auto;
        }
        pthread_join(sim->loop_thread, NULL);
        return 0;
    }
}

