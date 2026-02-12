#ifndef __UI_DYNO_H
#define __UI_DYNO_H

#include "ui.h"
#include "ui/ui_mod.h"
#include "libautodiag/com/obd/obd.h"
#include "libprog/config.h"
#include "libprog/ui/counter.h"
#include "libprog/ui/graph_data.h"
#include "ui/widget/menubar.h"

typedef struct {
    struct {
        struct {
            GtkMenuItem *all;
            GtkWidget *filter_by;
            GtkWidget *filter_by_menu;
        } data;
    } menuBar;
    GtkButton *toggle;
    GtkWindow *window;
    ErrorFeedbackWindows errorFeedback;
} DynoGui;

mod_gui * mod_gui_dyno_new();

#endif