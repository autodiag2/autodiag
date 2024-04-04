#ifndef __ERROR_FEEDBACK_H
#define __ERROR_FEEDBACK_H

#include "lib/lib.h"
#include "ui/gtk.h"
#include "com/serial/serial.h"
#include "com/obd/obd.h"

typedef struct {
    GtkMessageDialog * serial;
    GtkWidget * obd;
} ErrorFeedbackWindows;

#define ERROR_FEEDBACK_RETRIEVE_WINDOWS(builder) { \
    .serial = (GtkMessageDialog*) (gtk_builder_get_object (builder, "window-no-serial-selected")), \
    .obd = GTK_WIDGET (gtk_builder_get_object (builder, "window-no-obd-interface-openned")) \
}

void error_feedback_windows_init(final ErrorFeedbackWindows ws);
/**
 * @return true if a feedback has been returned else false
 */
bool error_feedback_serial(final ErrorFeedbackWindows ws, final SERIAL serial);
/**
 * Same but at a different level
 */
bool error_feedback_obd(final ErrorFeedbackWindows ws, final OBDIFace* iface, final SERIAL serial);

#endif
