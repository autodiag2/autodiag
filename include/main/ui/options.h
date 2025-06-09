#ifndef __OPTIONS_H
#define __OPTIONS_H

#define MODULE_OPTIONS "Options", 

#include "ui.h"
#include <stdlib.h>
#include "libautodiag/sim/elm327/sim.h"

typedef struct {
    GtkWidget *window;
    GtkComboBoxText *serialList;
    GtkEntry *baudRateSelection;
    GtkComboBoxText *logLevel;
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
    struct {
        GtkSpinner * spinner;
        GtkLabel *launchDesc;
        pthread_t * launchThread;
    } simulator;
} OptionsGui;

extern OptionsGui *optionsGui;

void module_init_options(GtkBuilder *builder);
void module_shutdown_options();
void options_show_window();
int ensure_serial_in_list(final Serial * port);

#endif
