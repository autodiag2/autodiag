#ifndef __VEHICULE_DIAGNOSTIC_H
#define __VEHICULE_DIAGNOSTIC_H

#define MODULE_VEHICULE_DIAGNOSTIC "Vehicle Diagnostic",

#include "ui.h"
#include "libautodiag/com/obd/obd.h"
#include "libprog/config.h"
#include "counter.h"
#include "libprog/graph_data.h"

#define VH_GEN_OX_SENSOR_STRUCT(i) \
    struct { \
        GtkLabel* voltage; \
        GtkLabel* current; \
        GtkLabel* ratio; \
    } sensor_##i;

/**
 * Structure that holds display of values on screen
 */
typedef struct {
    GtkWidget *window;
    GtkSpinner *refreshIcon;
    ErrorFeedbackWindows errorFeedback;
    struct {
        GtkCheckMenuItem *autoRefresh;
        pthread_t* freeze_frame_thread;
        GtkWindow* freeze_frame_error_popup;
        GtkCheckMenuItem *showFreezeFrame;
    } menuBar;
    struct {
        GtkBox* expandableSection;
        GtkProgressBar* speed;
        GtkProgressBar* load;
        GtkProgressBar* vehicleSpeed;
        GtkLabel* type;
        GtkLabel* secondsSinceStart;
        struct {
            GtkLabel* voltage;
            GtkLabel* obdStandard;
        } ecu;
        struct {
            GtkProgressBar* temperature;
            GtkProgressBar* manifoldPressure;
            GtkProgressBar* mafRate;
        } intakeAir;
        struct {
            GtkProgressBar* temperature;
        } coolant;
        struct {
            GtkProgressBar* pressure;
            GtkProgressBar* level;
            GtkLabel* status;
            GtkLabel* type;    
            GtkProgressBar* ethanol;        
            GtkProgressBar* rate;
            struct {
                struct {
                    GtkProgressBar* bank1, * bank2;
                } longTerm;
                struct {
                    GtkProgressBar* bank1, * bank2;
                } shortTerm;
            } trim;
            struct {
                GtkProgressBar* pressure;
            } rail;
        } fuel;
        struct {
            GtkProgressBar* injectionTiming;
            GtkProgressBar* timingAdvance;
        } injectionSystem;
        struct {
            VH_GEN_OX_SENSOR_STRUCT(1);
            VH_GEN_OX_SENSOR_STRUCT(2);
            VH_GEN_OX_SENSOR_STRUCT(3);
            VH_GEN_OX_SENSOR_STRUCT(4);
            VH_GEN_OX_SENSOR_STRUCT(5);
            VH_GEN_OX_SENSOR_STRUCT(6);
            VH_GEN_OX_SENSOR_STRUCT(7);
            VH_GEN_OX_SENSOR_STRUCT(8);
        } oxSensors;
        GtkContainer* tests;
    } engine;
    struct {
        GtkButton * add;
        GtkComboBoxText *list;
        GtkGrid * container;
        GtkButton * resetData;
    } graphs;
} vehicleExplorerGui;

void module_init_vehicle_explorer(final GtkBuilder *builder);

#define vehicle_explorer_error_feedback_obd(iface) \
 error_feedback_obd(vdgui->errorFeedback,iface,serial_list_get_selected())

#endif
