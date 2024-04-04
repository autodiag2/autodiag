#include "ui/error_feedback.h"

bool error_feedback_serial(final ErrorFeedbackWindows ws, final SERIAL serial) {
    char *msg = null;
    if ( serial == null ) {
        msg = strdup("No serial selected");
    } else {
        if ( serial->status != SERIAL_STATE_READY ) {
            asprintf(&msg, "Serial port in the wrong state : %s", serial_status_to_string(serial->status));
        }
    }
    if ( msg != null ) {
        log_msg(LOG_INFO, "Cannot read codes : serial in the wrong state");
        gtk_message_dialog_format_secondary_text(ws.serial,"%s",msg);
        free(msg);
        gtk_widget_show_on_main_thread(GTK_WIDGET(ws.serial));
    }
    return msg != null;
}

bool error_feedback_obd(final ErrorFeedbackWindows ws, final OBDIFace* iface, final SERIAL serial) {
    if ( iface == null ) {
        if ( ! error_feedback_serial(ws,serial) ) {
            log_msg(LOG_INFO, "Cannot read codes : no obd interface openned");
            gtk_widget_show_on_main_thread(GTK_WIDGET(ws.obd));
        }
        return true;
    } else {
        return error_feedback_serial(ws,(SERIAL)iface->device);
    }
}

void error_feedback_windows_init(final ErrorFeedbackWindows ws) {
    g_signal_connect(G_OBJECT(ws.obd),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
    g_signal_connect(G_OBJECT(ws.serial),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
}
