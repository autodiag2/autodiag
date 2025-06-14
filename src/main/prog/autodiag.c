#include "ui/main.h"
#include "libprog/serial_cli.h"
#include "libprog/elm327_cli.h"

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
           "  sim elm327 [elm327sim args] : run sim\n"
           "\n"
           " Gui order:\n"
           "  gui : run the gui\n"
           "\n"
           " Cli order:\n"
           "  cli [cli args] : run serial client\n"
           "\n"
    );
}

#define ABORT_WITH_HELP() \
    display_help(); \
    return 0;


gboolean autodiag_present_window(gpointer mainWindow) {
    sleep(1);
    gtk_window_set_keep_above(GTK_WINDOW(mainWindow), false);
    return false;
}
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
                printf("data path: %s\n", installation_folder(""));
                return 0;
            }
            ABORT_WITH_HELP()
        } else if argIs("sim") {
            argNext()
            if argIs("elm327") {
                return sim_elm327_cli_main(argc-argCurentIndex(), argv+argCurentIndex());
            }
           ABORT_WITH_HELP()
        } else if argIs("gui") {
            log_msg(LOG_INFO, "this is the default behaviour");
        } else if argIs("cli") {
            return serial_cli_main(argc-argCurentIndex(), argv+argCurentIndex());
        }
    }
    

    debug_show_config();
    log_msg(LOG_INFO, "Starting application ...");
    gtk_init (&argc, &argv);
    module_init_main();

    gtk_widget_show (mainGui->window);

    gtk_window_set_keep_above(GTK_WINDOW(mainGui->window), true);
    g_idle_add(autodiag_present_window, (gpointer)mainGui->window);
    gtk_window_present(GTK_WINDOW(mainGui->window));

    log_msg(LOG_INFO, "Application started");
    gtk_main();

    log_msg(LOG_INFO, "Application exit");

    return 0;
}
