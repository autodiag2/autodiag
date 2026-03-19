#include "ui/vin.h"
#include "mongoose.h"
#include "cJSON.h"
#include <time.h>

static gui_vin * gui = null;
static time_t last_query = 0;

struct vin_query_ctx {
    struct mg_mgr mgr;
    char *result;
    char *url;
    bool done;
    int status;
};
static void vin_http_cb(struct mg_connection *c, int ev, void *ev_data) {
    struct vin_query_ctx *ctx = (struct vin_query_ctx *) c->fn_data;

    if (ev == MG_EV_CONNECT) {
        int status = 0;
        if (ev_data != null) status = *(int *) ev_data;

        struct mg_tls_opts opts = {
            .ca = NULL,
            .name = "vpic.nhtsa.dot.gov"
        };
        mg_tls_init(c, &opts);
        
        if (status != 0) {
            ctx->done = true;
            ctx->status = -1;
            c->is_closing = 1;
            return;
        }

        // Manual host/path split
        const char *url = ctx->url;
        const char *p = strstr(url, "://");
        const char *host_start = url;
        const char *path_start = "/";
        if (p != NULL) host_start = p + 3;
        const char *slash = strchr(host_start, '/');
        char host[128], path[512];
        if (slash) {
            size_t len = slash - host_start;
            if (len >= sizeof(host)) len = sizeof(host)-1;
            memcpy(host, host_start, len);
            host[len] = 0;
            strncpy(path, slash, sizeof(path)-1);
            path[sizeof(path)-1] = 0;
        } else {
            strncpy(host, host_start, sizeof(host)-1);
            host[sizeof(host)-1] = 0;
            strcpy(path, "/");
        }

        mg_printf(c,
            "GET %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: autodiag/1.0\r\n"
            "Connection: close\r\n\r\n",
            path, host);
    }

    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        ctx->status = mg_http_status(hm);

        char *body = malloc(hm->body.len + 1);
        memcpy(body, hm->body.buf, hm->body.len);
        body[hm->body.len] = 0;

        ctx->result = body;
        ctx->done = true;
        c->is_closing = 1;
    }

    if (ev == MG_EV_ERROR) {
        ctx->status = -1;
        ctx->done = true;
        c->is_closing = 1;
    }
}

static char * vin_http_get(const char * url, int *status) {

    struct vin_query_ctx ctx;
    memset(&ctx, 0, sizeof(ctx));

    mg_mgr_init(&ctx.mgr);
    ctx.url = strdup(url);

    mg_http_connect(&ctx.mgr, url, vin_http_cb, &ctx);

    while ( ctx.done == false ) {
        mg_mgr_poll(&ctx.mgr, 100);
    }

    mg_mgr_free(&ctx.mgr);

    if ( status ) *status = ctx.status;

    return ctx.result;
}

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
    char *json = vin_http_get(url, &status);

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
    if ( ! uds_write_vin(config.ephemere.iface, ad_buffer_from_ascii(vin) ) ) {
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