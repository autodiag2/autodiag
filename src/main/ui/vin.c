#include "ui/vin.h"
#include "cJSON.h"
#include <time.h>

static gui_vin * gui = null;
static time_t last_query = 0;

static void button_click_clean_up_routine(void *arg) {

}

static void hide() {
    gtk_widget_hide_on_main_thread(gui->window);
}

static const char * vin_get() {
    return gtk_entry_get_text(gui->vin);
}

static int year_make_get() {
    const char * t = gtk_entry_get_text(gui->year_make);
    if ( t == null || strlen(t) == 0 ) return 0;
    return atoi(t);
}
static gboolean tool_year_make_set_gsource(gpointer data) {
    int year_make = *((int*)data);
    char * year_make_str = gprintf("%d", year_make);
    gtk_entry_set_text(gui->year_make, year_make_str);
    free(year_make_str);
    free(data);
    return false;
}
static void year_make_set(int year_make) {
    g_idle_add(tool_year_make_set_gsource, intdup(year_make));
}
static gboolean tool_vin_set_gsource(gpointer data) {
    char * text = (char*)data;
    gtk_entry_set_text(gui->vin, text);
    free(text);
    return false;
}
static void vin_set(char * vin) {
    g_idle_add(tool_vin_set_gsource, strdup(vin));
}
static gboolean tool_output_set_gsource(gpointer data) {
    char * text = (char*)data;
    gtk_text_buffer_set_text(gui->tool.output_buffer, text, strlen(text));
    free(text);
    return false;
}
static void tool_output_set(char * text) {
    g_idle_add(tool_output_set_gsource, strdup(text));
}
static gboolean tool_output_append_gsource(gpointer data) {
    char * text = (char*)data;

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(gui->tool.output_buffer, &end);
    gtk_text_buffer_insert(gui->tool.output_buffer, &end, text, strlen(text));

    free(text);
    return false;
}

static void tool_output_append(char * text) {
    g_idle_add(tool_output_append_gsource, strdup(text));
}
static bool ensure_vehicle_connected() {
    if ( config.ephemere.iface == null || config.ephemere.iface->vehicle == null || config.ephemere.iface->connection._state != VIFaceState_READY ) {
        tool_output_set("Vehicle not connected");
        return false;
    }
    return true;
}

static void tool_query_action() {

    time_t now = time(null);
    if ( now - last_query < 1 ) {
        tool_output_set("Rate limit: wait before querying again");
        return;
    }
    last_query = now;

    const char * vin = vin_get();
    int year = year_make_get();

    if ( vin == null || strlen(vin) < 5 ) {
        tool_output_set("Invalid VIN");
        return;
    }

    char url[512];

    if ( year > 0 ) {
        snprintf(
            url,
            sizeof(url),
            "https://vpic.nhtsa.dot.gov/api/vehicles/decodevinextended/%s?format=json&modelyear=%d",
            vin,
            year
        );
    } else {
        snprintf(
            url,
            sizeof(url),
            "https://vpic.nhtsa.dot.gov/api/vehicles/decodevinextended/%s?format=json",
            vin
        );
    }

    char err[256];
    int status;
    char *json = ad_http_get(url, &status);

    if ( json == null ) {
        tool_output_set("Network error");
        return;
    }

    if ( status != 200 ) {
        char buf[128];
        snprintf(buf, sizeof(buf), "HTTP error %d", status);
        tool_output_set(buf);
        return;
    }

    cJSON * root = cJSON_Parse(json);
    if ( root == null ) {
        tool_output_set("JSON parse error");
        free(json);
        return;
    }

    cJSON * results = cJSON_GetObjectItem(root, "Results");
    if ( !cJSON_IsArray(results) ) {
        tool_output_set("Unexpected API response");
        cJSON_Delete(root);
        free(json);
        return;
    }

    char * out = malloc(65536);
    out[0] = 0;

    int n = cJSON_GetArraySize(results);

    for ( int i = 0; i < n; i++ ) {

        cJSON * item = cJSON_GetArrayItem(results, i);

        cJSON * variable = cJSON_GetObjectItem(item, "Variable");
        cJSON * value = cJSON_GetObjectItem(item, "Value");

        if ( !cJSON_IsString(variable) ) continue;
        if ( value == null ) continue;
        if ( !cJSON_IsString(value) ) continue;

        if ( strlen(value->valuestring) == 0 ) continue;

        if ( strcmp(variable->valuestring, "ErrorCode") == 0 ) continue;
        if ( strcmp(variable->valuestring, "ErrorText") == 0 ) continue;

        strcat(out, variable->valuestring);
        strcat(out, " : ");
        strcat(out, value->valuestring);
        strcat(out, "\n");
    }

    if ( strlen(out) == 0 ) {
        strcpy(out, "No decoded fields");
    }

    tool_output_set(out);

    free(out);
    cJSON_Delete(root);
    free(json);
}
THREAD_WRITE_DAEMON(
    tool_query_daemon, tool_query_action,
    button_click_clean_up_routine, gui->tool.queryThread
)

static void tool_query_clicked(GtkButton *button, gpointer user_data) {
    thread_allocate_and_start(&gui->tool.queryThread, &tool_query_daemon);
}
static void tool_vin_read_action() {
    if ( ! ensure_vehicle_connected() ) return;

    char * vin = config.ephemere.iface->vehicle->vin == null ? strdup("") : ad_buffer_to_ascii_espace_breaking_chars(config.ephemere.iface->vehicle->vin);
    char * txt = strlen(vin) == 0 ? strdup("cannot retrieve vin") : gprintf("Reading VIN from this vehicle\nVIN : %s", vin);

    tool_output_set(txt);
    vin_set(vin);
    year_make_set(config.ephemere.iface->vehicle->year);

    free(txt);
    free(vin);
}

THREAD_WRITE_DAEMON(
    tool_vin_read_daemon, tool_vin_read_action,
    button_click_clean_up_routine, gui->tool.vinReadThread
)

static void tool_vin_read_clicked(GtkButton *button, gpointer user_data) {
    thread_allocate_and_start(&gui->tool.vinReadThread, &tool_vin_read_daemon);
}

static void tool_vin_write_action() {
    if ( ! ensure_vehicle_connected() ) return;
    char * vin = (char*)gtk_entry_get_text(gui->vin);
    if ( ! ad_uds_write_vin(config.ephemere.iface, ad_buffer_from_ascii(vin) ) ) {
        tool_output_set("Error while writting your vin, check format, check permission");
        return;
    }
    tool_output_set("VIN written, retrieve again informations from iface ...");
    config_onchange();
    tool_output_append("Informations loaded !");
}

THREAD_WRITE_DAEMON(
    tool_vin_write_daemon, tool_vin_write_action,
    button_click_clean_up_routine, gui->tool.vinWriteThread
)

static void tool_vin_write_clicked(GtkButton *button, gpointer user_data) {
    thread_allocate_and_start(&gui->tool.vinWriteThread, &tool_vin_write_daemon);
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