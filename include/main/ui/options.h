#ifndef __AD_OPTIONS_H
#define __AD_OPTIONS_H

#include "ui.h"
#include "ui_mod.h"
#include <stdlib.h>
#include "libautodiag/sim/ecu/ecu.h"
#include "libautodiag/model/database.h"
#include "libprog/sim_ecu_generator_gui.h"

typedef struct {
    GtkWidget *window;
    GtkComboBoxText *deviceList;
    GtkEntry *device_location;
    GtkComboBoxText *device_type;
    GtkEntry *baudRateSelection;
    GtkComboBoxText *logLevel;
    GtkCheckButton *recoveryMode;
    GtkCheckButton *connectAtStartup;
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
        struct {
            GtkCheckButton * enabled;
            GtkEntry * file;
            GtkFileChooserButton * fileChooser;
        } replay;
        GtkCheckButton * networkSim;
        GtkEntry * nvm_override;
        GtkButton * nvm_clear;
    } simulator;
    struct {
        GtkComboBoxText *manufacturer;
        GtkComboBoxText *engine;
        GtkEntry *vin;
    } vehicleInfos;
    struct {
        GtkCheckButton * enabled;
        GtkEntry * file;
        GtkFileChooserButton * fileChooser;
        GtkButton * export;
        GtkLabel * status;
    } recorder;

} OptionsGui;

mod_gui * mod_gui_options_new();

#endif
