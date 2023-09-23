#include "ui/error_feedback.h"

void error_feedback_serial(final GtkMessageDialog * dialog, final SERIAL serial) {
    log_msg("Cannot read codes : serial in the wrong state", LOG_INFO);
    char *msg;
    if ( serial == null ) {
        msg = strdup("No serial selected");
    } else {
        asprintf(&msg, "Serial port in the wrong state : %s", serial_status_to_string(serial->status));
    }
    gtk_message_dialog_format_secondary_text(dialog,"%s",msg);
    free(msg);
    gtk_widget_show_on_main_thread(GTK_WIDGET(dialog));
}
