#ifndef __ERROR_FEEDBACK_H
#define __ERROR_FEEDBACK_H

#include "libautodiag/lib.h"
#include "libprog/ui/gtk.h"
#include "libautodiag/com/serial/serial.h"
#include "libautodiag/com/obd/obd.h"

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
bool error_feedback_serial(final ErrorFeedbackWindows ws, final Serial * serial);
/**
 * Same but at a different level
 */
bool error_feedback_obd(final ErrorFeedbackWindows ws, final VehicleIFace* iface, final Serial * serial);

#endif
