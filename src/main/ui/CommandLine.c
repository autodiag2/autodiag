#include "ui/CommandLine.h"

#include <limits.h>
CommandLineGui *cmdGui = null;

void command_line_scroll_to_bottom() {
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(cmdGui->output.window);

    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_DOUBLE);
    g_value_set_double(&value, 0.0);
    g_object_get_property((GObject*)adj, "upper", &value);

    gtk_adjustment_set_value(adj, g_value_get_double(&value));
}

gboolean command_line_append_text_to_output_gsource(gpointer data) {
    final char *text = (char*)data;
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_offset(cmdGui->output.text,&iter,-1);
    gtk_text_buffer_insert(cmdGui->output.text, &iter, text, -1);
    if ( config.commandLine.autoScrollEnabled ) {
        command_line_scroll_to_bottom();
    }
    return false;
}

void command_line_append_text_to_output(final char *text) {
    g_idle_add(command_line_append_text_to_output_gsource, (gpointer)strdup(text));
}

void * command_line_send_command_wait_response_internal(final void * arg) {
    final char * command = (char*)arg;
    if ( 0 < strlen(command) ) {
        final SERIAL port = serial_list_get_selected();
        if ( port == null ) {
            error_feedback_serial(cmdGui->serialError.window,port);
        } else {
            buffer_recycle(port->recv_buffer);
            {
                char *ctime = config.commandLine.showTimestamp ? log_get_current_time() : strdup("");
                char msg[strlen(ctime) + 2 + strlen(command) + 1 + 1];
                sprintf(msg, "%s> %s\n", ctime, command);
                command_line_append_text_to_output(msg);
                free(ctime);
            }
            if ( serial_send(port, command) == SERIAL_INTERNAL_ERROR ) {
                error_feedback_serial(cmdGui->serialError.window,(SERIAL)port);                
            } else {
                serial_recv(port);
                {
                    final char * result = buffer_to_hexdump(port->recv_buffer->buffer, port->recv_buffer->size_used);
                    if ( result == null ) {
                        command_line_append_text_to_output("\n");
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
gboolean command_line_set_text_input_data(gpointer data) {
    char * command = (char*)data;
    gtk_entry_set_text(cmdGui->customCommandInput, command);
    free(command);
    return false;
}
gboolean command_line_set_text_tooltip_extra(gpointer data) {
    char * tooltip = (char*)data;
    gtk_label_set_text(cmdGui->tooltip, tooltip);
    free(tooltip);
    return false;
}
void command_line_generic_send_command_from_button(final GtkButton * button) {
    final char * command = (char*)gtk_button_get_label(button);
    g_idle_add(command_line_set_text_input_data, (gpointer)strdup(command));
    g_idle_add(command_line_set_text_tooltip_extra, (gpointer)strdup(gtk_widget_get_tooltip_text((GtkWidget*)button)));
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

void module_init_command_line(final GtkBuilder *builder) {
    if ( cmdGui == null ) {
        cmdGui = (CommandLineGui*)malloc(sizeof(CommandLineGui));
        CommandLineGui g = {
            .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-command-line")),
            .customCommandInput = (GtkEntry *)gtk_builder_get_object (builder, "window-command-line-custom-text"),
            .serialError = {
                .window = (GtkMessageDialog*) (gtk_builder_get_object (builder, "window-no-serial-selected"))
            },
            .output = {
                .window = (GtkScrolledWindow *)gtk_builder_get_object (builder, "window-command-line-terminal-output-window"),
                .frame = (GtkTextView *)gtk_builder_get_object (builder, "window-command-line-terminal-output"),
                .text = gtk_text_buffer_new(null)
            },
            .tooltip = (GtkLabel*)gtk_builder_get_object(builder,"window-command-line-tooltip")
        };
        *cmdGui = g;
        gtk_text_view_set_buffer(cmdGui->output.frame, cmdGui->output.text);
        g_signal_connect(G_OBJECT(cmdGui->window),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        g_signal_connect(G_OBJECT(cmdGui->serialError.window),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        gtk_builder_add_callback_symbol(builder,"show-window-command-line",&show_window_command_line);
        gtk_builder_add_callback_symbol(builder,"window-command-line-custom-send_clicked",&command_line_send_custom_command);
        gtk_builder_add_callback_symbol(builder,"window-command-line-clear_output_clicked",&command_line_output_clear);
        gtk_builder_add_callback_symbol(builder,"command_line_generic_send_command_from_button",G_CALLBACK(&command_line_generic_send_command_from_button));
    } else {
        module_debug(MODULE_COMMAND_LINE "Already initialized");        
    }
}

void module_shutdown_command_line() {
    if ( cmdGui != null ) {
        free(cmdGui);
    }
}


