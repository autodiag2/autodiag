#ifndef __OPTIONS_H
#define __OPTIONS_H

#define MODULE_OPTIONS "Options", 

#include "ui.h"
#include <stdlib.h>
#include "libautodiag/sim/elm327/sim_ecu.h"
#include "libautodiag/model/database.h"
#include "libprog/sim_ecu_generator_gui.h"

typedef struct {
    GtkWidget *window;
    GtkComboBoxText *serialList;
    GtkEntry *device_location;
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
        struct {
            GtkBox * container;
            GtkButton * add;
            GtkEntry * address;
            GtkComboBoxText * generator;
        } ecus;
    } simulator;
    struct {
        GtkComboBoxText *manufacturer;
        GtkComboBoxText *engine;
        GtkEntry *vin;
    } vehicleInfos;
} OptionsGui;


void module_init_options(GtkBuilder *builder);
void module_shutdown_options();

#endif
