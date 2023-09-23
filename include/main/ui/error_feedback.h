#ifndef __ERROR_FEEDBACK_H
#define __ERROR_FEEDBACK_H

#include "globals.h"
#include "gtk.h"
#include "com/serial/serial.h"

void error_feedback_serial(final GtkMessageDialog * dialog, final SERIAL serial);

#endif
