#include "ui/CommandLine.h"
#include "libautodiag/com/obdb/obdb.h"

#include <limits.h>

static CommandLineGui *gui = null;
static ad_list_ad_object_string * commandHistory = null;
static int commandHistoryIndex = -1;
static gdouble output_scrollbar_current_upper = -1;
static ad_object_vehicle_signal *selectedSignal = null;
static void command_line_signals_rebuild();

static gboolean choices_changed_gsource(gpointer data) {
    gtk_entry_set_text(gui->signals.obdb.registry, gtk_combo_box_text_get_active_text(gui->signals.obdb.choices));
    return false;
}
static void * choices_changed_daemon(void * arg) {
    g_idle_add(choices_changed_gsource, null);
    return null;
}
static void choices_changed(GtkComboBoxText *combo, gpointer user_data) {
    pthread_t t;
    pthread_create(&t, null, choices_changed_daemon, null);
}
static bool obdb_file_reader(Buffer *line, void *data) {
    GtkComboBoxText *cb = GTK_COMBO_BOX_TEXT(data);
    char * eol = strstr((char*)line->buffer, "\n");
    if ( eol ) {
        *eol = '\0';
    }
    char *text = strdup((char*)line->buffer);

    if (text != null && text[0] != 0) {
        gtk_combo_box_text_append_text(cb, text);
    }

    free(text);
    return true;
}

static void obdb_fill_choices(GtkComboBoxText *cb) {
    bool result;

    gtk_combo_box_text_remove_all(cb);

    result = file_read_lines(
        installation_folder_resolve("data/obdb-repositories.txt"),
        obdb_file_reader,
        cb
    );

    if (result) {
        gtk_widget_printf(GTK_WIDGET(gui->signals.obdb.status), "registries loaded");
    } else {
        gtk_widget_printf(GTK_WIDGET(gui->signals.obdb.status), "failed to load registries");
    }
}
static void output_scrollbar_size_changed(GtkAdjustment *adj, gpointer user_data) {
    if ( config.commandLine.autoScrollEnabled ) {
        gdouble upper = gtk_adjustment_get_upper(adj);
        if ( output_scrollbar_current_upper != upper ) {
            output_scrollbar_current_upper = upper;
            gdouble v = upper - gtk_adjustment_get_page_size(adj);
            gtk_adjustment_set_value(adj,v);
        }
    }
}
static void command_line_signal_output_hide() {
    gtk_widget_hide(GTK_WIDGET(gui->signals.output.container));
    gtk_label_set_text(gui->signals.output.value, "");
    gtk_label_set_text(gui->signals.output.examples, "");
}
static void command_line_signal_selection_clear() {
    selectedSignal = null;
    command_line_signal_output_hide();
}

static void command_line_signal_selection_set(ad_object_vehicle_signal *signal) {
    selectedSignal = signal;
}
static void command_line_signal_output_show_value(ad_object_vehicle_signal *signal, double value) {
    char buf[256];
    const char *unit = signal != null && signal->unit != null ? signal->unit : "";
    snprintf(buf, sizeof(buf), "%.17g%s%s", value, unit[0] != 0 ? " " : "", unit);
    gtk_label_set_text(gui->signals.output.value, buf);
    gtk_label_set_text(gui->signals.output.examples, signal->examples ? signal->examples : "");
    gtk_widget_show(GTK_WIDGET(gui->signals.output.container));
}
static gboolean input_line_keypress(GtkWidget *entry, GdkEventKey  *event, gpointer user_data) {
    if (commandHistory->size == 0)
        return FALSE;

    switch (event->keyval) {
        case GDK_KEY_Up:
            commandHistoryIndex ++;
            if ( commandHistoryIndex == commandHistory->size ) {
                commandHistoryIndex--;
            } else {
                gtk_entry_set_text(GTK_ENTRY(entry), commandHistory->list[commandHistory->size-1-commandHistoryIndex]->data);
                gtk_label_set_text(gui->tooltip, "");
                gtk_editable_set_position(GTK_EDITABLE(entry), -1);
            }
            return TRUE;

        case GDK_KEY_Down:
            commandHistoryIndex --;
            if ( commandHistoryIndex < -1 ) {
                commandHistoryIndex++;
            } else if ( commandHistoryIndex == -1 ) {
                gtk_entry_set_text(GTK_ENTRY(entry), "");
                gtk_label_set_text(gui->tooltip, "");
            } else {
                gtk_entry_set_text(GTK_ENTRY(entry), commandHistory->list[commandHistory->size-1-commandHistoryIndex]->data);
                gtk_label_set_text(gui->tooltip, "");
                gtk_editable_set_position(GTK_EDITABLE(entry), -1);
            }
            return TRUE;

        default:
            return FALSE;
    }
}

static gboolean append_text_to_output_gsource(gpointer data) {
    final char *text = (char*)data;
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_offset(gui->output.text,&iter,-1);
    gtk_text_buffer_insert(gui->output.text, &iter, text, -1);
    return false;
}

static void append_text_to_output(final char *text) {
    g_idle_add(append_text_to_output_gsource, (gpointer)strdup(text));
}
#include <ctype.h>
static char *ascii_interpret_escape_sequences(const char *input, int * output_len_rv) {
    int output_len = 0;
    const char *src = input;
    char *parsed = malloc(strlen(input) + 1);
    char *dst = parsed;
    while (*src) {
        if (*src == '\\') {
            src++;
            if (*src == 'x' && isxdigit(src[1]) && isxdigit(src[2])) {
                char hex[3] = { src[1], src[2], 0 };
                *dst++ = strtol(hex, NULL, 16);
                src += 3;
                output_len ++;
            } else if (*src == 'r') {
                *dst++ = '\r';
                src++;
                output_len ++;
            } else if (*src == 'n') {
                *dst++ = '\n';
                src++;
                output_len ++;
            } else if (*src == 't') {
                *dst++ = '\t';
                src++;
                output_len ++;
            } else {
                *dst++ = *src++;
                output_len ++;
            }
        } else {
            *dst++ = *src++;
            output_len ++;
        }
    }
    *dst = '\0';
    if ( output_len_rv != null ) {
        *output_len_rv = output_len;
    }
    return parsed;
}
static void on_send_command_any(char * command) {
    char *ctime = config.commandLine.showTimestamp ? log_get_current_time() : strdup("");
    char msg[strlen(ctime) + 2 + strlen(command) + 1 + 1];
    sprintf(msg, "%s> %s\n", ctime, command);
    append_text_to_output(msg);
    free(ctime);
}
static void on_send_command_reply_any(Buffer * buffer) {
    final char * result = bytes_to_hexdump(buffer->buffer, buffer->size);
    if ( result == null ) {
        append_text_to_output("No data received from the device\n");
    } else {
        append_text_to_output(result);
        free(result);
    }   
}
static void * send_command_wait_response_internal(final void * arg) {
    char * command = (char*)arg;
    final Device * device = device_table_get_selected(config.ephemere.device_table);
    if ( ! error_feedback_device(gui->errorFeedback,device) ) {
        device->lock(AD_DEVICE(device));
        ad_buffer_recycle(device->recv_buffer);
        on_send_command_any(command);
        int command_len = strlen(command);
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->send.interpretEscapes))) {
            char *tmp = ascii_interpret_escape_sequences(command, &command_len);
            command = tmp;
        }
        final int result;
        if ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->send.raw)) ) {
            switch( device->type ) {
                case AD_DEVICE_TYPE_SERIAL: {
                    Serial * serial = (Serial*) device;
                    result = serial_send_internal(serial, command, command_len);
                } break;
                case AD_DEVICE_TYPE_DOIP: {
                    ad_object_DoIPDevice * doip = (ad_object_DoIPDevice*)device;
                    result = doip_send_internal(doip, command, command_len);
                } break;
                default: {
                    result = DEVICE_ERROR; 
                    log_msg(LOG_DEBUG, "TODO");
                } break;
            }
        } else {
            result = device->send(AD_DEVICE(device), command);
        }
        if ( result == DEVICE_ERROR ) {
            error_feedback_device(gui->errorFeedback,device);                
        } else {
            device->recv(AD_DEVICE(device));
            on_send_command_reply_any(device->recv_buffer);
        }
        device->unlock(AD_DEVICE(device));
    }
    pthread_exit(0);
}

static void send_command_wait_response(final char * command) {
    pthread_t t;
    pthread_create(&t, null, &send_command_wait_response_internal, command);
}
static gboolean send_command_from_button_gsource(gpointer button) {
    final char * command = (char*)gtk_button_get_label((GtkButton*)button);
    gtk_entry_set_text(gui->customCommandInput, command);
    command_line_signal_selection_clear();

    char *tooltipText = gtk_widget_get_tooltip_text((GtkWidget*)button);
    if ( tooltipText == null ) {
        tooltipText = strdup("");
        log_msg(LOG_WARNING, "No tooltip text for button with label '%s'", command);
    }
    gtk_label_set_text(gui->tooltip, tooltipText);
    return false;
}
void command_line_generic_send_command_from_button(final GtkButton * button) {
    g_idle_add(send_command_from_button_gsource, (gpointer)button);
}
static void send_custom_command() {
    final char * command = (char *)gtk_entry_get_text(gui->customCommandInput);

    if ( -1 == commandHistoryIndex || strcmp(commandHistory->list[commandHistory->size-1-commandHistoryIndex]->data, command) != 0 ) {
        if ( 0 == commandHistory->size || strcmp(commandHistory->list[commandHistory->size-1]->data, command) != 0 ) {
            if ( 0 < strlen(command) ) {
                ad_list_ad_object_string_append(commandHistory, ad_object_string_new_from(command));
            }
        }
    }

    if (selectedSignal != null) {
        double value = 0.0;
        bool ok;

        if (config.ephemere.iface == null) {
            command_line_signal_output_hide();
            gtk_label_set_text(gui->tooltip, "No active vehicle interface");
            return;
        }
        if ( error_feedback_obd(gui->errorFeedback, config.ephemere.iface, config.ephemere.iface->device) ) {
            return;
        }
        
        ad_object_vehicle_signal * custom_signal = ad_object_vehicle_signal_copy(selectedSignal);
        custom_signal->input_formula = strdup(command);
        on_send_command_any(custom_signal->input_formula);
        ok = viface_use_signal(config.ephemere.iface, custom_signal, &value, null);
        on_send_command_reply_any(config.ephemere.iface->device->recv_buffer);
        if (!ok) {
            gtk_widget_show(GTK_WIDGET(gui->signals.output.container));
            gtk_label_set_text(gui->signals.output.value, "Error");
            gtk_label_set_text(gui->signals.output.examples, custom_signal->examples ? custom_signal->examples : "");
            return;
        }

        command_line_signal_output_show_value(custom_signal, value);
        ad_object_vehicle_signal_free(custom_signal);
    } else {
        command_line_signal_output_hide();
        send_command_wait_response(strdup(command));
        gtk_entry_set_text(gui->customCommandInput, "");
    }

    commandHistoryIndex = -1;
}

static void output_clear() {
    gtk_text_buffer_set_text(gui->output.text, "", 0);
}

static void custom_clear() {
    gtk_entry_set_text(gui->customCommandInput,"");
    gtk_label_set_text(gui->tooltip,"");
    command_line_signal_selection_clear();
    commandHistoryIndex = -1;   
}

static void output_copy() {
    GtkTextIter start, end;

    gtk_text_buffer_get_start_iter(gui->output.text, &start);
    gtk_text_buffer_get_end_iter(gui->output.text, &end);
                                
    gchar *text = gtk_text_buffer_get_text(gui->output.text, &start, &end, true);

    GdkDisplay *dpy = gdk_display_get_default();
    GtkClipboard *clipboard = gtk_clipboard_get_default(dpy);
    gtk_clipboard_set_text(clipboard, text, strlen(text));
    log_msg(LOG_INFO, "text \"%s\" copied", text);
}

static bool vehicle_set_tooltip_text(GtkButton* child) {
    const gchar* text = gtk_button_get_label(child);
    bool res = false;
    final Buffer* bin_buffer = ad_buffer_from_ascii_hex((char*)text);
    gchar *tooltipText = null;
    if ( 1 < bin_buffer->size ) {
        switch(bin_buffer->buffer[0]) {
            case 0x01: tooltipText = saej1979_data_pid_code_to_str(bin_buffer->buffer[1]);
        }
        if ( tooltipText != null ) {
            gtk_widget_set_tooltip_text(GTK_WIDGET(child),tooltipText);
            res = true;
            free(tooltipText);
        }
    }
    ad_buffer_free(bin_buffer);
    return res;
}
typedef struct {
    GtkWidget *root;
    GtkWidget *groups_box;
    GHashTable *groups;
    const char *query;
} CommandLineSignalsBuildCtx;

static void widget_destroy_if_not_self(GtkWidget *widget, gpointer data) {
    GtkWidget *self = GTK_WIDGET(data);
    if (widget != self) {
        gtk_widget_destroy(widget);
    }
}

static void box_remove_all_children(GtkWidget *box) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(box));
    for (GList *it = children; it != null; it = it->next) {
        gtk_widget_destroy(GTK_WIDGET(it->data));
    }
    g_list_free(children);
}

static gboolean string_contains_case_insensitive(const char *haystack, const char *needle) {
    gchar *h;
    gchar *n;
    gboolean ok;

    if (needle == null || needle[0] == 0) {
        return TRUE;
    }
    if (haystack == null) {
        return FALSE;
    }

    h = g_ascii_strdown(haystack, -1);
    n = g_ascii_strdown(needle, -1);
    ok = strstr(h, n) != null;
    g_free(h);
    g_free(n);
    return ok;
}

static gboolean signal_matches_search(ad_object_vehicle_signal *signal, const char *query) {
    if (query == null || query[0] == 0) {
        return TRUE;
    }
    return string_contains_case_insensitive(signal->name, query)
        || string_contains_case_insensitive(signal->description, query)
        || string_contains_case_insensitive(signal->slug, query)
        || string_contains_case_insensitive(signal->standard, query);
}

static void command_line_signal_fill(ad_object_vehicle_signal *signal) {
    if (signal == null) {
        return;
    }

    gtk_entry_set_text(gui->customCommandInput, signal->input_formula != null ? signal->input_formula : "");
    gtk_label_set_text(gui->tooltip, signal->description != null ? signal->description : "");
    gtk_widget_grab_focus(GTK_WIDGET(gui->customCommandInput));
    gtk_editable_set_position(GTK_EDITABLE(gui->customCommandInput), -1);
}
static gboolean obdb_fetch_clicked_gsource(gpointer data) {
    if ( *((bool*)data) ) {
        command_line_signals_rebuild();
        gtk_label_set_text(gui->signals.obdb.status, "fetch success");
    } else {
        gtk_label_set_text(gui->signals.obdb.status, "fetch error");
    }
    return false;
}
static void * obdb_fetch_daemon(void *arg) {
    bool result = ad_obdb_fetch_signals((char*)arg);
    g_idle_add(obdb_fetch_clicked_gsource, booldup(result));
    return null;
}
static void obdb_fetch_clicked(GtkButton *button, gpointer userdata) {
    pthread_t t;
    pthread_create(&t, null, obdb_fetch_daemon, (char*)gtk_entry_get_text(gui->signals.obdb.registry));
}
static void command_line_signal_button_clicked(GtkButton *button, gpointer userdata) {
    ad_object_vehicle_signal *signal = (ad_object_vehicle_signal*)userdata;
    double value = 0.0;
    bool ok;

    command_line_signal_fill(signal);
    command_line_signal_selection_set(signal);

    if (signal == null) {
        command_line_signal_output_hide();
        return;
    }
    if (config.ephemere.iface == null) {
        command_line_signal_output_hide();
        gtk_label_set_text(gui->tooltip, "No active vehicle interface");
        return;
    }

    ok = viface_use_signal(config.ephemere.iface, signal, &value, null);
    if (!ok) {
        gtk_widget_show(GTK_WIDGET(gui->signals.output.container));
        gtk_label_set_text(gui->signals.output.value, "Error");
        gtk_label_set_text(gui->signals.output.examples, signal->examples ? signal->examples : "");
        return;
    }

    command_line_signal_output_show_value(signal, value);
}

static GtkWidget *command_line_signals_get_or_create_group(CommandLineSignalsBuildCtx *ctx, const char *standard) {
    GtkWidget *group_box;
    GtkWidget *frame;
    GtkWidget *inner;

    group_box = g_hash_table_lookup(ctx->groups, standard);
    if (group_box != null) {
        return group_box;
    }

    frame = gtk_frame_new(standard != null ? standard : AD_OBJECT_VEHICLE_SIGNAL_NO_STANDARD);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_hexpand(frame, TRUE);

    inner = gtk_flow_box_new();
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(inner), GTK_SELECTION_NONE);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(inner), INT_MAX);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(inner), 4);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(inner), 4);
    gtk_widget_set_hexpand(inner, TRUE);
    gtk_widget_set_vexpand(inner, FALSE);

    gtk_container_add(GTK_CONTAINER(frame), inner);

    gtk_box_pack_start(GTK_BOX(ctx->groups_box), frame, TRUE, TRUE, 4);
    gtk_widget_show(frame);
    gtk_widget_show(inner);

    g_hash_table_insert(
        ctx->groups,
        g_strdup(standard != null ? standard : AD_OBJECT_VEHICLE_SIGNAL_NO_STANDARD),
        inner
    );
    return inner;
}

static void command_line_signals_add_one(ad_object_vehicle_signal *signal, void *userdata) {
    CommandLineSignalsBuildCtx *ctx = (CommandLineSignalsBuildCtx*)userdata;
    GtkWidget *group_box;
    GtkWidget *button;

    if (signal == null) {
        return;
    }
    if (!signal_matches_search(signal, ctx->query)) {
        return;
    }

    group_box = command_line_signals_get_or_create_group(ctx, signal->standard);
    button = gtk_button_new_with_label(signal->name != null ? signal->name : "(unnamed signal)");

    if (signal->description != null) {
        gtk_widget_set_tooltip_text(button, signal->description);
    }

    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(command_line_signal_button_clicked), signal);
    gtk_widget_set_halign(button, GTK_ALIGN_START);
    gtk_widget_set_hexpand(button, FALSE);
    gtk_flow_box_insert(GTK_FLOW_BOX(group_box), button, -1);
    gtk_widget_show(button);
}

static void command_line_signals_rebuild() {
    CommandLineSignalsBuildCtx ctx;

    box_remove_all_children(GTK_WIDGET(gui->signals.categories));

    ctx.root = GTK_WIDGET(gui->signals.categories);
    ctx.groups_box = GTK_WIDGET(gui->signals.categories);
    ctx.groups = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, null);
    ctx.query = gtk_entry_get_text(GTK_ENTRY(gui->signals.search));

    ad_signal_foreach(command_line_signals_add_one, &ctx);

    g_hash_table_destroy(ctx.groups);
}

static void command_line_signals_search_changed(GtkEditable *editable, gpointer userdata) {
    command_line_signals_rebuild();
}
static void init(final GtkBuilder *builder) {
    if ( gui == null ) {
        commandHistory = ad_list_ad_object_string_new();
        gui = (CommandLineGui*)malloc(sizeof(CommandLineGui));
        CommandLineGui g = {
            .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-command-line")),
            .customCommandInput = (GtkEntry *)gtk_builder_get_object (builder, "window-command-line-custom-text"),
            .errorFeedback = ERROR_FEEDBACK_RETRIEVE_WINDOWS(builder),
            .output = {
                .window = (GtkScrolledWindow *)gtk_builder_get_object (builder, "window-command-line-terminal-output-window"),
                .frame = (GtkTextView *)gtk_builder_get_object (builder, "window-command-line-terminal-output"),
                .text = gtk_text_buffer_new(null)
            },
            .tooltip = (GtkLabel*)gtk_builder_get_object(builder,"window-command-line-tooltip"),
            .vehicleOBDCodes = (GtkGrid*)gtk_builder_get_object(builder,"window-command-line-vehicle-obd-codes"),
            .send = {
                .interpretEscapes = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "command-line-interpret-escapes")),
                .raw = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "command-line-raw"))
            },
            .signals = {
                .search = GTK_SEARCH_ENTRY(gtk_builder_get_object(builder, "command-line-signals-search")),
                .categories = GTK_BOX(gtk_builder_get_object(builder, "command-line-signals-categories")),
                .output = {
                    .container = GTK_BOX(gtk_builder_get_object(builder, "command-line-signal-return-container")),
                    .value = GTK_LABEL(gtk_builder_get_object(builder, "command-line-signal-return")),
                    .examples = GTK_LABEL(gtk_builder_get_object(builder, "command-line-signal-examples"))
                },
                .obdb = {
                    .registry = GTK_ENTRY(gtk_builder_get_object(builder, "command-line-signals-obdb-registry")),
                    .fetch = GTK_BUTTON(gtk_builder_get_object(builder, "command-line-signals-obdb-fetch")),
                    .status = GTK_LABEL(gtk_builder_get_object(builder, "command-line-signals-obdb-error")),
                    .choices = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "command-line-signals-choices"))
                }
            }
        };
        *gui = g;
        command_line_signal_output_hide();
        obdb_fill_choices(gui->signals.obdb.choices);
        assert(0 != g_signal_connect(gui->signals.obdb.choices, "changed", G_CALLBACK(choices_changed), null));

        g_signal_connect(G_OBJECT(gui->signals.obdb.fetch), "clicked", G_CALLBACK(obdb_fetch_clicked), null);
        assert(0 != g_signal_connect(G_OBJECT(gui->signals.search), "changed", G_CALLBACK(command_line_signals_search_changed), null));
        command_line_signals_rebuild();
        gtk_text_view_set_buffer(gui->output.frame, gui->output.text);
        assert(0 != g_signal_connect(G_OBJECT(gui->window),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null));
        assert(0 != g_signal_connect(gui->customCommandInput, "key-press-event", G_CALLBACK(input_line_keypress), NULL));
        error_feedback_windows_init(gui->errorFeedback);
        assert(0 != g_signal_connect(G_OBJECT(gtk_scrolled_window_get_vadjustment(gui->output.window)),"changed",G_CALLBACK(output_scrollbar_size_changed),null));        
        gtk_builder_add_callback_symbol(builder,"window-command-line-custom-send_clicked",&send_custom_command);
        gtk_builder_add_callback_symbol(builder,"window-command-line-clear_output_clicked",&output_clear);
        gtk_builder_add_callback_symbol(builder,"send_command_from_button",G_CALLBACK(&command_line_generic_send_command_from_button));
        gtk_builder_add_callback_symbol(builder,"window-command-line-output-copy",&output_copy);
        gtk_builder_add_callback_symbol(builder,"window-command-line-custom-clear-clicked",&custom_clear);
    } else {
        log_msg(LOG_DEBUG, "Already initialized");        
    }
}

static void end() {
    if ( gui != null ) {
        free(gui);
        gui = null;
    }
}

static void show() {
    gtk_window_show_ensure_ontop(GTK_WIDGET(gui->window));
    gtk_widget_grab_focus(GTK_WIDGET(gui->customCommandInput));
}

static void hide() {
    gtk_widget_hide_on_main_thread(GTK_WIDGET(gui->window));
}

mod_gui * mod_gui_command_line_new() {
    mod_gui * mg = (mod_gui*)malloc(sizeof(mod_gui));
    mg->init = init;
    mg->end = end;
    mg->name = strdup("Command Line");
    mg->show = show;
    mg->hide = hide;
    return mg;
}