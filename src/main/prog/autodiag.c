#include "ui/main.h"

int main (int argc, char *argv[]) {
    debug_show_config();
    log_set_from_env();
    log_msg(LOG_INFO, "Starting application ...");
    gtk_init (&argc, &argv);
    module_init_main();

    gtk_widget_show (mainGui->window);
    log_msg(LOG_INFO, "Application started");
    gtk_main();

    log_msg(LOG_INFO, "Application exit");
    return 0;
}
