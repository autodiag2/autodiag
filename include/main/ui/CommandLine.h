#ifndef __COMMAND_LINE_H
#define __COMMAND_LINE_H

#define MODULE_COMMAND_LINE "CommandLine",

#include "ui.h"
#include "com/serial/serial.h"
#include "com/serial/serial_list.h"
#include "lib/buffer.h"
#include "config.h"

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
} CommandLineGui;

extern CommandLineGui *cmdGui;

void command_line_generic_send_command_from_button(final GtkButton * button);
void module_init_command_line(GtkBuilder *builder);
void module_shutdown_command_line();
#endif
