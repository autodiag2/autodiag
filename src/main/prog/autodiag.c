#include "ui/main.h"
#include "libprog/serial_cli.h"
#include "libprog/elm327_cli.h"
#include "libprog/doip_cli.h"

static void display_help() {
    printf(
           "\n"
           "Autodiag\n"
           "  autodiag [order, params]\n"
           "\n"
           " Common order:\n"
           "  help : display help\n"
           "\n"
           " Program files order:\n"
           "  config path  : display config path\n"
           "  config reset : reset config to default\n"
           "  data path    : display app data\n"
           "\n"
           " Simulation order:\n"
           "  sim <type> [sim args] : run sim\n"
           "\n"
           " Gui order:\n"
           "  gui [gui args] : run the gui\n"
           "\n"
           " Cli order:\n"
           "  cli [cli args] : run serial client\n"
           "\n"
           " Params:\n"
           " -l [log level]    : set or list the log level\n"
           "\n"
           "Example:\n"
           " autodiag sim elm327\n"
           " autodiag sim doip\n"
           " autodiag gui\n"
           "\n"
    );
}
static void display_gui_help() {
    printf(
    "\n"
    " -h                : display this help\n"
    " -b <baud rate>    : set the baud rate\n"
    " -n [name]         : device location (comport, pseudo terminals, named pipes)\n"
    "\n"
    );
}

#define ABORT_WITH_HELP() \
    display_help(); \
    return 0;

int main (int argc, char *argv[]) {

    log_set_from_env();
    config_init();
    config_load();
    serial_table_fill(config.ephemere.device_table);
    
    if ( ! log_is_env_set() ) {
        log_set_level(config.log.level);
    }
    
    argForEach() {
        if ( argIs("help") || argIs("--help") || argIs("-h") ) {
            ABORT_WITH_HELP()
        } else if argIs("config") {
            argNext()
            if argIs("path") {
                printf("config path: %s\n", config_get_config_filename());
                return 0;
            } else if argIs("reset") {
                return ! config_reset();
            }
            ABORT_WITH_HELP()
        } else if argIs("data") {
            argNext()
            if argIs("path") {
                printf("data path: %s\n", installation_folder_resolve(""));
                return 0;
            }
            ABORT_WITH_HELP()
        } else if argIs("sim") {
            argNext()
            char * arg = argCurrent();
            if ( arg == null ) {
                printf("Available sims:\n");
                printf(" elm327\n");
                printf(" doip\n");
                return 0;
            } else if argIs("elm327") {
                return sim_elm327_cli_main(argc-argCurrentIndex(), argv+argCurrentIndex());
            } else if argIs("doip") {
                return sim_doip_cli_main(argc-argCurrentIndex(), argv+argCurrentIndex());
            }
           ABORT_WITH_HELP()
        } else if argIs("gui") {
            log_msg(LOG_INFO, "this is the default behaviour");
            argNext()
            if ( argIs("-h") || argIs("help") || argIs("--help") ) {
                display_gui_help();
                return 0;
            } else if argIs("-b") {
                argNext();
                char * arg = argCurrent();
                if ( arg == null ) {
                    printf("-b require the baud rate as argument\n");
                    return 1;
                } else {
                    config.com.serial.baud_rate = atoi(arg);
                }
            } else if argIs("-n") {
                argNext();
                char * arg = argCurrent();
                if ( arg == null ) {
                    printf("Devices available:\n");
                    for(int i = 0; i < config.ephemere.device_table->list->size; i++) {
                        Device * device = AD_DEVICE(config.ephemere.device_table->list->list[i]);
                        printf(" %s (%s)\n", device->location, device_type_as_string(device->type));
                    }
                    return 0;
                } else {
                    config.com.serial.device_location = strdup(arg);
                }
            } else {
                printf("Unknown argument '%s', aborting\n", argCurrent());
                return 1;
            }
        } else if argIs("cli") {
            return serial_cli_main(argc-argCurrentIndex(), argv+argCurrentIndex());
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
                config.log.level = log_level_from_str(arg);
                log_set_level(config.log.level);
            }  
        } else {
            printf("Unknown order or arg: '%s'\n", argCurrent());
            return 1;
        }
    }
    

    debug_show_config();
    log_msg(LOG_INFO, "Starting application ...");
    gtk_init (&argc, &argv);
    module_init_main();

    gtk_window_show_ensure_ontop(mainGui->window);

    log_msg(LOG_INFO, "Application started");
    gtk_main();

    log_msg(LOG_INFO, "Application exit");

    return 0;
}
