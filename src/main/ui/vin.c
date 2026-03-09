#include "ui/vin.h"

static gui_vin * gui = null;

static void hide() {
    gtk_widget_hide_on_main_thread(gui->window);
}

static const char * vin_get() {
    return gtk_entry_get_text(gui->vin);
}

static void year_make_set(int year_make) {
    char * year_make_str = gprintf("%d", year_make);
    gtk_entry_set_text(gui->year_make, year_make_str);
    free(year_make_str);
}
static void vin_set(char * vin) {
    gtk_entry_set_text(gui->vin, vin);
}
static void tool_output_set(char * text) {
    gtk_text_buffer_set_text(gui->tool.output_buffer, text, strlen(text));
}
static void tool_query_clicked(GtkButton *button, gpointer user_data) {
    tool_output_set("TODO");
    // vin_get
    // year_make_get
}
static void tool_vin_read_clicked(GtkButton *button, gpointer user_data) {
    char * vin = ad_buffer_to_ascii_espace_breaking_chars(config.ephemere.iface->vehicle->vin);
    char * txt = gprintf("Reading VIN from this vehicle\nVIN : %s", vin);
    tool_output_set(txt);
    vin_set(vin);
    year_make_set(config.ephemere.iface->vehicle->year);
    free(txt);
}
static void tool_vin_write_clicked(GtkButton *button, gpointer user_data) {
    tool_output_set("Not implemented ...");
}
static void init(final GtkBuilder * builder) {
    if ( gui ) {
        log_msg(LOG_WARNING, "mod already init");
        return;
    }

    gui_vin g = {
        .window = GTK_WIDGET(gtk_builder_get_object(builder, "window-vin")),
        .vin = GTK_ENTRY(gtk_builder_get_object(builder, "vin-entry")),
        .year_make = GTK_ENTRY(gtk_builder_get_object(builder, "vin-year-make")),
        .tool = {
            .query = GTK_BUTTON(gtk_builder_get_object(builder, "vin-query")),
            .vinRead = GTK_BUTTON(gtk_builder_get_object(builder, "vin-read")),
            .vinWrite = GTK_BUTTON(gtk_builder_get_object(builder, "vin-write")),
            .output = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "vin-tool-output")),
            .output_buffer = gtk_text_buffer_new(null),
        }
    };
    gtk_text_view_set_buffer(g.tool.output, g.tool.output_buffer);
    gui = (gui_vin*)malloc(sizeof(gui_vin));
    *gui = g;
    assert(0 != g_signal_connect(G_OBJECT(gui->window), "delete-event", G_CALLBACK(hide), null));
    assert(0 != g_signal_connect(g.tool.query, "clicked", G_CALLBACK(tool_query_clicked), null));
    assert(0 != g_signal_connect(g.tool.vinRead, "clicked", G_CALLBACK(tool_vin_read_clicked), null));
    assert(0 != g_signal_connect(g.tool.vinWrite, "clicked", G_CALLBACK(tool_vin_write_clicked), null));
}

static void end() {
    if ( gui != null ) {
        free(gui);
        gui = null;
    }
}

static void show() {
    gtk_window_show_ensure_ontop(gui->window);
}

mod_gui * mod_gui_vin_new() {
    mod_gui * mg = (mod_gui*)malloc(sizeof(mod_gui));
    mg->init = init;
    mg->end = end;
    mg->name = strdup("VIN tools");
    mg->show = show;
    mg->hide = hide;
    return mg;
}