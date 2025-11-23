#include "ui/lib/gtk_wrapper.h"

void gtk_wrapper_set_window_icon(GtkWindow * window) {
    char * mediaDir = installation_folder_resolve("media");
    if ( mediaDir == null ) {
        log_msg(LOG_ERROR, "Data directory not found try to reinstall the software");
        exit(1);
    }
    char * iconPath;
    asprintf(&iconPath, "%s" PATH_FOLDER_DELIM "app_icon.png", mediaDir);
    gtk_window_set_icon_from_file(GTK_WINDOW(window), iconPath, NULL);
    free(iconPath);
}