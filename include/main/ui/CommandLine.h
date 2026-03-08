#ifndef __AD_COMMAND_LINE_H
#define __AD_COMMAND_LINE_H

#include "ui.h"
#include "ui_mod.h"
#include "libautodiag/com/serial/serial.h"
#include "libautodiag/com/doip/doip.h"
#include "libautodiag/com/device_table.h"
#include "libautodiag/buffer.h"
#include "libprog/config.h"

typedef struct {
    GtkWidget *window;
    GtkEntry *customCommandInput;
    GtkLabel *tooltip;
    GtkGrid* vehicleOBDCodes;
    ErrorFeedbackWindows errorFeedback;
    struct {
        GtkScrolledWindow *window;
        GtkTextView *frame;
        GtkTextBuffer *text;
    } output;
    struct {
        GtkCheckButton * interpretEscapes;
        GtkCheckButton * raw;
    } send;
} CommandLineGui;

void command_line_generic_send_command_from_button(final GtkButton * button);
mod_gui * mod_gui_command_line_new();

#endif
