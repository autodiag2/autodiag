#ifndef __VEHICULE_DIAGNOSTIC_H
#define __VEHICULE_DIAGNOSTIC_H

#define MODULE_VEHICULE_DIAGNOSTIC "Vehicle Diagnostic",

#include "globals.h"
#include <gtk/gtk.h>
#include "log.h"
#include "com/serial/obd/saej1979/saej1979.h"

/**
 * Structure that holds display of values on screen
 */
typedef struct {
    GtkWidget *window;
    GtkSpinner *refreshIcon;
    struct {
        GtkBox* expandableSection;
        GtkProgressBar* speed;
        GtkLabel* type;
        struct {
            GtkLabel* voltage;
            GtkLabel* obdStandard;
        } ecu;
        struct {
            GtkProgressBar* temperature;
        } intakeAir;
        struct {
            GtkProgressBar* temperature;
        } coolant;
        struct {
            GtkProgressBar* pressure;
            GtkProgressBar* level;
            struct {
                struct {
                    GtkProgressBar* bank1, * bank2;
                } longTerm;
                struct {
                    GtkProgressBar* bank1, * bank2;
                } shortTerm;
            } trim;
        } fuel;
    } engine;
} vehicleExplorerGui;

void module_init_vehicle_explorer(final GtkBuilder *builder);

#endif
