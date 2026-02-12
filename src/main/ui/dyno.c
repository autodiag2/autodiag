#include "ui/dyno.h"

static DynoGui *gui = null;

MENUBAR_DATA_ALL_IN_ONE

static gboolean onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused) {
    gtk_widget_hide(GTK_WIDGET(gui->window));
    return true;
}

static void init(final GtkBuilder *builder) {
    if (gui != null) {
        log_msg(LOG_WARNING, "Already init dyno mod");
        return;
    }

    DynoGui g = {
        .window = GTK_WINDOW(gtk_builder_get_object(builder, "dyno-window")),
        .menuBar = {
            .data = {
                .all = GTK_MENU_ITEM(gtk_builder_get_object(builder, "dyno-menubar-data-all")),
                .filter_by = GTK_WIDGET(gtk_builder_get_object(builder, "dyno-menubar-data-filter-by"))
            }
        },
        .errorFeedback = ERROR_FEEDBACK_RETRIEVE_WINDOWS(builder)
    };

    gui = (DynoGui*)malloc(sizeof(DynoGui));
    (*gui) = g;

    assert(0 != g_signal_connect(G_OBJECT(gui->window), "delete-event", G_CALLBACK(onclose), NULL));

    error_feedback_windows_init(gui->errorFeedback);
    
    MENUBAR_DATA_CONNECT()
}

static void end() {
    if ( gui != null ) {
        free(gui);
        gui = null;
    }
}

static void show() {
    gtk_window_show_ensure_ontop(GTK_WIDGET(gui->window));
}

static void hide() {

}

mod_gui * mod_gui_dyno_new() {
    mod_gui * mg = (mod_gui*)malloc(sizeof(mod_gui));
    mg->init = init;
    mg->end = end;
    mg->name = strdup("Dynamometer");
    mg->show = show;
    mg->hide = hide;
    return mg;
}