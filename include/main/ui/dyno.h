#ifndef __AD_UI_DYNO_H
#define __AD_UI_DYNO_H

#include "ui.h"
#include "ui/ui_mod.h"
#include "libautodiag/com/obd/obd.h"
#include "libprog/config.h"
#include "libprog/ui/counter.h"
#include "libprog/ui/graph_data.h"
#include "ui/widget/menubar.h"
#include "libautodiag/thread.h"

typedef struct {
    struct {
        struct {
            GtkMenuItem *all;
            GtkWidget *filter_by;
            GtkWidget *filter_by_menu;
        } data;
    } menuBar;

    GtkWindow *window;

    ErrorFeedbackWindows errorFeedback;

    struct {
        GtkLabel *lbl_state;
        GtkLabel *lbl_time;
        GtkLabel *lbl_speed;
        GtkLabel *lbl_rpm;
        GtkLabel *lbl_pwr;
        GtkLabel *lbl_hp;
        GtkLabel *lbl_tq;
        GtkSpinner *spinner;
    } status;

    struct {
        GtkLabel *lbl_time;
        GtkLabel *lbl_speed;
        GtkLabel *lbl_rpm;
        GtkLabel *lbl_pwr;
        GtkLabel *lbl_hp;
        GtkLabel *lbl_tq;
    } peak;

    struct {
        GtkSpinButton *mass_kg;
        GtkSpinButton *sample_hz;
    } params;

    struct {
        GtkButton *btn_start;
        GtkButton *btn_stop;
        GtkButton *btn_reset;
    } actions;

} DynoGui;

mod_gui *mod_gui_dyno_new();

#define dyno_error_feedback_obd(iface) \
 error_feedback_obd(gui->errorFeedback,iface,device_table_get_selected(config.ephemere.device_table))

#endif