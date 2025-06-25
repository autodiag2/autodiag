#ifndef __COMMAND_LINE_H
#define __COMMAND_LINE_H

#define MODULE_COMMAND_LINE "CommandLine",

#include "ui.h"
#include "libautodiag/com/serial/serial.h"
#include "libautodiag/com/serial/list_serial.h"
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
void module_init_command_line(GtkBuilder *builder);
void module_shutdown_command_line();
#endif
