#include "ui/main.h"

void display_help() {
    printf(
           "\n"
           "Autodiag\n"
           "  autodiag [order]\n"
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
           "  sim elm327 help : display sim help\n"
           "  sim elm327 run  : run sim\n"
           "\n"
           " Gui order:\n"
           "  gui run : run the gui\n"
           "\n"
    );
}

#define ABORT_WITH_HELP() \
    display_help(); \
    return 0;


int main (int argc, char *argv[]) {

    log_set_from_env();
    
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
                printf("data path: %s\n", config_get_in_data_folder_safe(""));
                return 0;
            }
            ABORT_WITH_HELP()
        } else if argIs("sim") {
            argNext()
            if argIs("elm327") {
                argNext()
                if argIs("help") {
                    elm327_sim_cli_display_help();
                    return 0;
                } else if argIs("run") {
                    return elm327_sim_cli_main(argc, argv, argCurentIndex());
                }
            }
           ABORT_WITH_HELP()
        } else if argIs("gui") {
            argNext()
            if argIs("run") {
                log_msg(LOG_INFO, "this is the default behaviour");
            } else {
                ABORT_WITH_HELP()
            }
        }
    }
    

    debug_show_config();
    log_msg(LOG_INFO, "Starting application ...");
    gtk_init (&argc, &argv);
    module_init_main();

    gtk_widget_show (mainGui->window);
    log_msg(LOG_INFO, "Application started");
    gtk_main();

    log_msg(LOG_INFO, "Application exit");

    return 0;
}
