#ifndef __AD_UI_VIN_H
#define __AD_UI_VIN_H

#include "ui/ui.h"
#include "ui/ui_mod.h"
#include "libprog/config.h"

typedef struct {
    GtkWidget * window;
    GtkEntry * vin;
    GtkEntry * year_make;
    struct {
        GtkButton * query;
        GtkButton * vinRead;
        GtkButton * vinWrite;
        GtkTextView * output;
        GtkTextBuffer * output_buffer;
    } tool;
} gui_vin;

mod_gui * mod_gui_vin_new();

#endif