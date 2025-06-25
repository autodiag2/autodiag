#include "ui/CommandLine.h"

#include <limits.h>

static CommandLineGui *gui = null;
static list_object_string * commandHistory = null;
static int commandHistoryIndex = -1;
static gdouble output_scrollbar_current_upper = -1;
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
static char *ascii_interpret_escape_sequences(const char *input) {
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
            } else if (*src == 'r') {
                *dst++ = '\r';
                src++;
            } else if (*src == 'n') {
                *dst++ = '\n';
                src++;
            } else if (*src == 't') {
                *dst++ = '\t';
                src++;
            } else {
                *dst++ = *src++;
            }
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    return parsed;
}
static void * send_command_wait_response_internal(final void * arg) {
    char * command = (char*)arg;
    final Serial * port = list_serial_get_selected();
    if ( ! error_feedback_serial(gui->errorFeedback,port) ) {
        buffer_recycle(port->recv_buffer);
        {
            char *ctime = config.commandLine.showTimestamp ? log_get_current_time() : strdup("");
            char msg[strlen(ctime) + 2 + strlen(command) + 1 + 1];
            sprintf(msg, "%s> %s\n", ctime, command);
            append_text_to_output(msg);
            free(ctime);
        }
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->send.interpretEscapes))) {
            char *tmp = ascii_interpret_escape_sequences(command);
            command = tmp;
        }
        final int result;
        if ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->send.raw)) ) {
            result = serial_send_internal(port, command, strlen(command));
        } else {
            result = port->send(CAST_DEVICE(port), command);
        }
        if ( result == DEVICE_ERROR ) {
            error_feedback_serial(gui->errorFeedback,port);                
        } else {
            port->recv(CAST_DEVICE(port));
            {
                final char * result = bytes_to_hexdump(port->recv_buffer->buffer, port->recv_buffer->size);
                if ( result == null ) {
                    append_text_to_output("No data received from the device\n");
                } else {
                    append_text_to_output(result);
                    free(result);
                }
            }
        }
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

static void show() {
   gtk_widget_show_now (gui->window);
   gtk_widget_grab_focus(GTK_WIDGET(gui->customCommandInput));
}
static void send_custom_command() {
    final char * command = (char *)gtk_entry_get_text(gui->customCommandInput);
    if ( -1 == commandHistoryIndex || strcmp(commandHistory->list[commandHistory->size-1-commandHistoryIndex]->data, command) != 0 ) {
        if ( 0 == commandHistory->size || strcmp(commandHistory->list[commandHistory->size-1]->data, command) != 0 ) {
            list_object_string_append(commandHistory, object_string_new_from(command));
        }
    }
    send_command_wait_response(strdup(command));
    commandHistoryIndex = -1;
    gtk_entry_set_text(gui->customCommandInput, "");
}

static void output_clear() {
    gtk_text_buffer_set_text(gui->output.text, "", 0);
}

static void custom_clear() {
    gtk_entry_set_text(gui->customCommandInput,"");
    gtk_label_set_text(gui->tooltip,"");    
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
    final Buffer* bin_buffer = buffer_from_ascii_hex((char*)text);
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
    buffer_free(bin_buffer);
    return res;
}

static void vehicle_add_pid_buttons() {
    // insert at the end
    int rowi = 1000;
    int rowSize = 10;
    for(int pid = 0; pid < 0xC0; pid++) {
        char *label;
        asprintf(&label,"01%02X",pid);
        GtkWidget* widget = GTK_WIDGET(gtk_button_new_with_label(label));
        if ( vehicle_set_tooltip_text(GTK_BUTTON(widget)) ) {
            g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(command_line_generic_send_command_from_button),null);
            int columni = pid % rowSize;
            if ( columni == 0 ) {
                gtk_grid_insert_row (gui->vehicleOBDCodes, ++rowi);
            }          
            gtk_grid_attach(gui->vehicleOBDCodes,
                     widget,
                     columni,
                     rowi,
                     1,1);
            gtk_widget_show(widget);
        } else {
            gtk_widget_destroy(widget);
        }
        free(label);
    }
}

void module_init_command_line(final GtkBuilder *builder) {
    if ( gui == null ) {
        commandHistory = list_object_string_new();
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
            }
        };
        *gui = g;
        vehicle_add_pid_buttons();
        gtk_text_view_set_buffer(gui->output.frame, gui->output.text);
        g_signal_connect(G_OBJECT(gui->window),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        g_signal_connect(gui->customCommandInput, "key-press-event", G_CALLBACK(input_line_keypress), NULL);
        error_feedback_windows_init(gui->errorFeedback);
        g_signal_connect(G_OBJECT(gtk_scrolled_window_get_vadjustment(gui->output.window)),"changed",G_CALLBACK(output_scrollbar_size_changed),null);        
        gtk_builder_add_callback_symbol(builder,"show-window-command-line",&show);
        gtk_builder_add_callback_symbol(builder,"window-command-line-custom-send_clicked",&send_custom_command);
        gtk_builder_add_callback_symbol(builder,"window-command-line-clear_output_clicked",&output_clear);
        gtk_builder_add_callback_symbol(builder,"send_command_from_button",G_CALLBACK(&command_line_generic_send_command_from_button));
        gtk_builder_add_callback_symbol(builder,"window-command-line-output-copy",&output_copy);
        gtk_builder_add_callback_symbol(builder,"window-command-line-custom-clear-clicked",&custom_clear);
    } else {
        module_debug(MODULE_COMMAND_LINE "Already initialized");        
    }
}

void module_shutdown_command_line() {
    if ( gui != null ) {
        free(gui);
    }
}