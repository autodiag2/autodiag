#ifndef __OPTIONS_H
#define __OPTIONS_H

#define MODULE_OPTIONS "Options", 

#define _GNU_SOURCE
#include <stdio.h>
#include <gtk/gtk.h>
#include "log.h"
#include "com/serial/obd/obd.h"

typedef struct {
    GtkWidget *window;
    GtkComboBoxText *serialList;
    GtkComboBoxText *baudRateSelection;
    struct {
        GtkToggleButton * advancedLinkDetails;
    } mainGui;
    struct {
        GtkToggleButton * outputAutoScroll;
        GtkToggleButton * showTimestamp;
    } commandLineGui;
    struct {
        GtkEntry * refreshRateS;
    } vehicleExplorerGui;
} OptionsGui;

extern OptionsGui *optionsGui;

void module_init_options(GtkBuilder *builder);
void module_shutdown_options();
void options_show_window();
int ensure_serial_in_list(final Serial * port);

#endif
