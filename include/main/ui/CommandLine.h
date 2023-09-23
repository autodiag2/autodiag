#ifndef __COMMAND_LINE_H
#define __COMMAND_LINE_H

#define MODULE_COMMAND_LINE "CommandLine",

#include "globals.h"
#include "com/serial/serial.h"
#include "com/serial/serial_list.h"
#include "buffer.h"
#include "log.h"
#include "error_feedback.h"
#include "gtk.h"
#include "config.h"

typedef struct {
    GtkWidget *window;
    GtkEntry *customCommandInput;
    GtkLabel *tooltip;
    struct serialError {
        GtkMessageDialog *window;
    } serialError;
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
