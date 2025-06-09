#include "ui/CommandLine.h"

#include <limits.h>
CommandLineGui *cmdGui = null;

gdouble command_line_output_scrollbar_current_upper = -1;
void command_line_output_scrollbar_size_changed(GtkAdjustment *adj, gpointer user_data) {
    if ( config.commandLine.autoScrollEnabled ) {
        gdouble upper = gtk_adjustment_get_upper(adj);
        if ( command_line_output_scrollbar_current_upper != upper ) {
            command_line_output_scrollbar_current_upper = upper;
            gdouble v = upper - gtk_adjustment_get_page_size(adj);
            gtk_adjustment_set_value(adj,v);
        }
    }
}

gboolean command_line_append_text_to_output_gsource(gpointer data) {
    final char *text = (char*)data;
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_offset(cmdGui->output.text,&iter,-1);
    gtk_text_buffer_insert(cmdGui->output.text, &iter, text, -1);
    return false;
}

void command_line_append_text_to_output(final char *text) {
    g_idle_add(command_line_append_text_to_output_gsource, (gpointer)strdup(text));
}

void * command_line_send_command_wait_response_internal(final void * arg) {
    final char * command = (char*)arg;
    if ( 0 < strlen(command) ) {
        final SERIAL port = serial_list_get_selected();
        if ( ! error_feedback_serial(cmdGui->errorFeedback,port) ) {
            buffer_recycle(port->recv_buffer);
            {
                char *ctime = config.commandLine.showTimestamp ? log_get_current_time() : strdup("");
                char msg[strlen(ctime) + 2 + strlen(command) + 1 + 1];
                sprintf(msg, "%s> %s\n", ctime, command);
                command_line_append_text_to_output(msg);
                free(ctime);
            }
            if ( port->send(port, command) == DEVICE_ERROR ) {
                error_feedback_serial(cmdGui->errorFeedback,port);                
            } else {
                port->recv(port);
                {
                    final char * result = bytes_to_hexdump(port->recv_buffer->buffer, port->recv_buffer->size);
                    if ( result == null ) {
                        command_line_append_text_to_output("No data received from the device\n");
                    } else {
                        command_line_append_text_to_output(result);
                        free(result);
                    }
                }
            }
        }
    } else {
        module_debug(MODULE_COMMAND_LINE "void command detected");
    }
    pthread_exit(0);
}

void command_line_send_command_wait_response(final char * command) {
    pthread_t t;
    pthread_create(&t, null, &command_line_send_command_wait_response_internal, command);
}
gboolean command_line_generic_send_command_from_button_gsource(gpointer button) {
    final char * command = (char*)gtk_button_get_label((GtkButton*)button);
    gtk_entry_set_text(cmdGui->customCommandInput, command);

    char *tooltipText = gtk_widget_get_tooltip_text((GtkWidget*)button);
    if ( tooltipText == null ) {
        tooltipText = strdup("");
        log_msg(LOG_WARNING, "No tooltip text for button with label '%s'", command);
    }
    gtk_label_set_text(cmdGui->tooltip, tooltipText);
    return false;
}
void command_line_generic_send_command_from_button(final GtkButton * button) {
    g_idle_add(command_line_generic_send_command_from_button_gsource, (gpointer)button);
}

void show_window_command_line() {
   gtk_widget_show_now (cmdGui->window);
   gtk_widget_grab_focus(GTK_WIDGET(cmdGui->customCommandInput));
}
void command_line_send_custom_command() {
    final char * command = (char *)gtk_entry_get_text(cmdGui->customCommandInput);
    command_line_send_command_wait_response(command);
}

void command_line_output_clear() {
    gtk_text_buffer_set_text(cmdGui->output.text, "", 0);
}

void command_line_custom_clear() {
    gtk_entry_set_text(cmdGui->customCommandInput,"");
    gtk_label_set_text(cmdGui->tooltip,"");    
}

void command_line_output_copy() {
    GtkTextIter start, end;

    gtk_text_buffer_get_start_iter(cmdGui->output.text, &start);
    gtk_text_buffer_get_end_iter(cmdGui->output.text, &end);
                                
    gchar *text = gtk_text_buffer_get_text(cmdGui->output.text, &start, &end, true);

    GdkDisplay *dpy = gdk_display_get_default();
    GtkClipboard *clipboard = gtk_clipboard_get_default(dpy);
    gtk_clipboard_set_text(clipboard, text, strlen(text));
    log_msg(LOG_INFO, "text \"%s\" copied", text);
}

bool command_line_vehicle_set_tooltip_text(GtkButton* child) {
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

void command_line_vehicle_add_pid_buttons() {
    // insert at the end
    int rowi = 1000;
    int rowSize = 10;
    for(int pid = 0; pid < 0xC0; pid++) {
        char *label;
        asprintf(&label,"01%02X",pid);
        GtkWidget* widget = GTK_WIDGET(gtk_button_new_with_label(label));
        if ( command_line_vehicle_set_tooltip_text(GTK_BUTTON(widget)) ) {
            g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(command_line_generic_send_command_from_button),null);
            int columni = pid % rowSize;
            if ( columni == 0 ) {
                gtk_grid_insert_row (cmdGui->vehicleOBDCodes, ++rowi);
            }          
            gtk_grid_attach(cmdGui->vehicleOBDCodes,
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
    if ( cmdGui == null ) {
        cmdGui = (CommandLineGui*)malloc(sizeof(CommandLineGui));
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
            .vehicleOBDCodes = (GtkGrid*)gtk_builder_get_object(builder,"window-command-line-vehicle-obd-codes")
        };
        *cmdGui = g;
        command_line_vehicle_add_pid_buttons();
        gtk_text_view_set_buffer(cmdGui->output.frame, cmdGui->output.text);
        g_signal_connect(G_OBJECT(cmdGui->window),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        error_feedback_windows_init(cmdGui->errorFeedback);
        g_signal_connect(G_OBJECT(gtk_scrolled_window_get_vadjustment(cmdGui->output.window)),"changed",G_CALLBACK(command_line_output_scrollbar_size_changed),null);        
        gtk_builder_add_callback_symbol(builder,"show-window-command-line",&show_window_command_line);
        gtk_builder_add_callback_symbol(builder,"window-command-line-custom-send_clicked",&command_line_send_custom_command);
        gtk_builder_add_callback_symbol(builder,"window-command-line-clear_output_clicked",&command_line_output_clear);
        gtk_builder_add_callback_symbol(builder,"command_line_generic_send_command_from_button",G_CALLBACK(&command_line_generic_send_command_from_button));
        gtk_builder_add_callback_symbol(builder,"window-command-line-output-copy",&command_line_output_copy);
        gtk_builder_add_callback_symbol(builder,"window-command-line-custom-clear-clicked",&command_line_custom_clear);
    } else {
        module_debug(MODULE_COMMAND_LINE "Already initialized");        
    }
}

void module_shutdown_command_line() {
    if ( cmdGui != null ) {
        free(cmdGui);
    }
}


