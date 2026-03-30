#ifndef __AD_UI_VIN_H
#define __AD_UI_VIN_H

#include "ui/ui.h"
#include "ui/ui_mod.h"
#include "libprog/config.h"
#include "libautodiag/com/uds/uds.h"
#include "libautodiag/com/http/http.h"

typedef struct {
    GtkWidget * window;
    GtkEntry * vin;
    GtkEntry * year_make;
    struct {
        GtkButton * query;
        pthread_t * queryThread;
        GtkButton * vinRead;
        pthread_t * vinReadThread;
        GtkButton * vinWrite;
        pthread_t * vinWriteThread;
        GtkTextView * output;
        GtkTextBuffer * output_buffer;
    } tool;
} gui_vin;

mod_gui * mod_gui_vin_new();

#endif