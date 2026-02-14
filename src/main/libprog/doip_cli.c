#include "libprog/doip_cli.h"

PRINT_MODULAR(sim_doip_cli_help,
    "\n"
    "DoIP simulator\n"
    "\n"
    " -h                        : display this help\n"
    " -e hh                     : add an ecu to the simulation with address hh\n"
    " -l                        : list level of logging\n"
    " -l level                  : set level of logging\n"
    " -g                        : list available generators\n"
    " -g generator              : set the generator of values\n"
    " -c context                : context for the generator\n"
    " --load-from-json jsoncxt  : clear previously defined simulation definition and\n"
    "                             and load ecus, generators from the json provided (text, filepath)\n"
    "\n"
    "Examples:\n"
    " doipsim -g cycle -e EA -g random    : default ecu E8 cycle generator, EA ecu random generator\n"
    " doipsim -g random -c 1234           : use the seed 1234 for generating random numbers\n"
    " doipsim -g cycle -c 10              : number of gears used in the cycle\n"
#ifndef COMPILE_COMPAT
    " doipsim -g gui                      : default ecu with gui value generator\n"
#endif
    "\n"
)

static void display_help() {
    sim_doip_cli_help("");
}

static void *launch(void *d) {
    SimDoIp * sim = (SimDoIp*)d;
    sim_doip_loop_as_daemon(sim);
    sim_doip_loop_daemon_wait_ready(sim);
    if ( sim->device_location == null ) {
        log_msg(LOG_WARNING, "Simulation not started");
    } else {
        printf("%s\n", sim->device_location);
        if ( sim->implementation->loop_thread != null ) {
            pthread_join(*sim->implementation->loop_thread, NULL);
        }
    }
    return null;
}
int sim_doip_cli_main(int argc, char **argv) {
    SimDoIp* sim = sim_doip_new();
    #ifndef COMPILE_COMPAT
        list_SimECUGeneratorGui * guis = list_SimECUGeneratorGui_new();
    #endif

    argForEach() {
        if ( argIs("-h") || argIs("--help") || argIs("help") ) {
            display_help();
            return 0;
        } else if argIs("-e") {
            argNext();
            char * arg = argCurrent();
            byte ecu_address;
            if ( arg == null ) {
                printf("example: -e E8\n"); 
                return 0;
            } else if ( sscanf(arg,"%02hhx", &ecu_address) == 1 ) {
                list_SimECU_append(sim->ecus,sim_ecu_new(ecu_address));                    
            } else {
                display_help();
                return 1;
            }
        } else if argIs("-l") {
            argNext();
            char * arg = argCurrent();
            if ( arg == null ) {
                printf("log levels:\n");
                printf("none\n");
                printf("error\n");                        
                printf("warning\n");                        
                printf("info\n");                        
                printf("debug\n");
                return 0;
            } else {
                logger.current_level = log_level_from_str(arg);
            }
        } else if argIs("-g") {
            argNext();
            char * arg = argCurrent();
            assert(0 < LIST_SIM_ECU(sim->ecus)->size);
            final SimECU * target_ecu = LIST_SIM_ECU(sim->ecus)->list[LIST_SIM_ECU(sim->ecus)->size - 1];
            final SimECUGenerator * generator;
            if ( arg == null ) {
                printf("Available generators:\n");
                printf("random\n");
                printf("cycle\n");
                printf("citroen_c5_x7\n");
                #ifndef COMPILE_COMPAT
                    printf("gui\n");
                #endif
                printf("replay\n");
                return 0;
            } else if ( strcasecmp(arg, "random") == 0 ) {
                generator = sim_ecu_generator_new_random();
            } else if ( strcasecmp(arg, "cycle") == 0 ) {
                generator = sim_ecu_generator_new_cycle();
            } else if ( strcasecmp(arg,"citroen_c5_x7") == 0 ) {
                generator = sim_ecu_generator_new_citroen_c5_x7();
            } else if (strcasecmp(arg,"replay") == 0 ) {
                generator = sim_ecu_generator_new_replay();
            #ifndef COMPILE_COMPAT
                } else if ( strcasecmp(arg,"gui") == 0 ) {
                    generator = sim_ecu_generator_new_gui();
                    char address[3];
                    sprintf(address, "%02hhX", target_ecu->address);
                    list_SimECUGeneratorGui_append(guis, 
                        sim_ecu_generator_gui_set_context(generator, address)
                    );
            #endif
            } else {
                log_msg(LOG_ERROR, "Unknown generator %s, rerun without argument to see available generators", arg);
                return 1;
            }
            generator->type = strdup(arg);
            target_ecu->generator = generator;
        } else if argIs("-c") {
            argNext();
            char * arg = argCurrent();
            assert(0 < LIST_SIM_ECU(sim->ecus)->size);
            final SimECUGenerator *generator = LIST_SIM_ECU(sim->ecus)->list[LIST_SIM_ECU(sim->ecus)->size - 1]->generator;
            if ( arg == null ) {
                printf("give the context to the -c, cannot be empty\n");
                return 0;
            } else if ( strcasecmp(generator->type, "random") == 0 ) {
                unsigned *context = (unsigned*)malloc(sizeof(unsigned));
                generator->context = context; 
                if ( sscanf(arg, "%u", context) != 1 ) {
                    printf("Expected unsigned int context for random generator, context is used as the seed\n");
                    return 1;
                }
            } else if ( strcasecmp(generator->type, "cycle") == 0 ) {
                unsigned *context = (unsigned*)malloc(sizeof(unsigned));
                generator->context = context; 
                if ( sscanf(arg, "%u", context) != 1 ) {
                    printf("Expected unsigned int number of gears for cycle of generator\n");
                    return 1;
                }
            } else if ( strcasecmp(generator->type, "replay") == 0 ) {
                generator->context = strdup(arg);
            } else {
                printf("Generator type %s as no user definable context\n", generator->type);
                return 1;
            }
        } else if argIs("--load-from-json") {
            argNext();
            char * arg = argCurrent();
            if ( arg == null ) {
                printf("Need to provide the json context as argument\n");
                return 1;
            }
            if ( sim_load_from_json(SIM(sim), arg) == GENERIC_FUNCTION_ERROR ) {
                printf("Failed to load from json, check your json\n");
                return 1;
            }
        } else {
            printf("Unknown arg '%s', aborting\n", argCurrent());
            return 1;
        }
    }
    #ifndef COMPILE_COMPAT
        for(int i = 0; i < guis->size; i ++) {
            SimECUGeneratorGui *simGui = guis->list[i];
            sim_ecu_generator_gui_show(simGui);
        }
    #endif

    pthread_t simThread;
    pthread_create(&simThread, null, &launch, sim);

    #ifdef COMPILE_COMPAT
        pthread_join(simThread, null);
    #else
        if ( 0 < guis->size ) {
            gtk_main();
        } else {
            pthread_join(simThread, null);
        }
        list_SimECUGeneratorGui_free(guis);
    #endif

    return 0;
}