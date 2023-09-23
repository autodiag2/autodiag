#include "ui/main.h"

MainGui *mainGui = null;

void module_shutdown_main() {
    module_shutdown_serial();
    module_shutdown_options();
    module_shutdown_command_line();
    gtk_main_quit();
}

gboolean main_onclose(final GtkWidget *dialog, final GdkEvent *event, gpointer unused) {
    module_debug(MODULE_MAIN "Close event received");
    module_shutdown_main();
    return true;
}

void debug_show_config() {
   log_msg("=== Running configuration ===", LOG_INFO);
   #ifdef POSIX_TERMIOS
      log_msg("POSIX Standard: Termios", LOG_INFO);
   #else
      unsupported_configuration();
   #endif
   log_msg("=============================", LOG_INFO);
}

void adaptater_state_set_text(final char * str, final char * color) {
    final const char *format = "<span foreground=\"%s\">\%s</span>";
    final char *markup = g_markup_printf_escaped (format, color, str);
    gtk_label_set_markup (GTK_LABEL (mainGui->adaptater.state.text), markup);
    g_free(markup);
}

void adaptater_protocol_set_text(final char * str) {
    final const char *format = "<span>\%s</span>";
    final char *markup = g_markup_printf_escaped (format, str);
    gtk_label_set_markup (GTK_LABEL (mainGui->adaptater.state.more.protocole), markup);
    g_free(markup);
}
void adaptater_interface_set_text(final char * str) {
    final const char *format = "<span>\%s</span>";
    final char *markup = g_markup_printf_escaped (format, str);
    gtk_label_set_markup (GTK_LABEL (mainGui->adaptater.state.more.interface), markup);
    g_free(markup);
}

void*refresh_usb_adaptater_state_spinner(void *arg) {
    final pthread_t * self = (pthread_t*)arg;
    gtk_spinner_start(mainGui->adaptater.state.waitIcon);
    pthread_join(*self, NULL);
    gtk_spinner_stop(mainGui->adaptater.state.waitIcon);
    free(self);
    pthread_exit(0);
}

void* refresh_usb_adaptater_state_internal(void *arg) {
    final SERIAL port = serial_list_get_selected();
    if ( port == null ) {
        adaptater_state_set_text("No serial port selected", "orange");
    } else {
        switch(port->status) {
            case SERIAL_STATE_READY:
                adaptater_state_set_text("Ready", "green");
                break;
            case SERIAL_STATE_NOT_OPEN:
                adaptater_state_set_text("Not open", "orange");
                break;
            case SERIAL_STATE_OPEN_ERROR:
                adaptater_state_set_text("Error while opening serial port (eg permissions not sufficient)", "red");
                break;
            case SERIAL_STATE_DISCONNECTED:
                adaptater_state_set_text("Serial port disconnected", "orange");
                break;
            case SERIAL_STATE_MISSING_PERM:
                adaptater_state_set_text("Missing permissions", "red");
                break;
            default:
                adaptater_state_set_text("No data", "red");
                break;
        }
        final OBDIFACE iface = config.ephemere.iface;
        if ( iface == null ) {
            adaptater_protocol_set_text("Not an OBD interface");
            adaptater_interface_set_text("Not an OBD interface");
        } else {
            adaptater_protocol_set_text((char*)obd_get_protocol_string(iface->type, iface->protocol));
            adaptater_interface_set_text((char*)serial_at_port_interface_to_id(iface->type));
        }
    }
    gtk_widget_set_visible(GTK_WIDGET(mainGui->adaptater.state.more.container), config.main.adaptater_detailled_settings_showned);
    pthread_exit(0);
}

void main_usb_adaptater_state_spinner_wait_for(void* (*function)(void*arg)) {
    pthread_t *t,t1;
    t = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(t, null, function, null);
    pthread_create(&t1, null, &refresh_usb_adaptater_state_spinner, t);    
}

void refresh_usb_adaptater_state() {
    main_usb_adaptater_state_spinner_wait_for(&refresh_usb_adaptater_state_internal);
}

void * module_init_main_deferred(void *ignored) {
    config_onchange();
    refresh_usb_adaptater_state_internal(null);
}

void module_init_main() {
    if ( mainGui == null ) {
        config_load();
        final GtkBuilder *builder = gtk_builder_new();
        char * config_directory = config_get_data_directory_safe();
        char * mainUiPath;
        asprintf(&mainUiPath, "%s/ui/main.glade", config_directory);
        free(config_directory);
        if ( gtk_builder_add_from_file (builder, mainUiPath, NULL) == 0 ) {
            log_msg("Error during ui inflation", LOG_ERROR);
        }
        free(mainUiPath);

        final MainGui gui = {
            .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-root")),
            .adaptater = {
                .state = {
                    .text = (GtkLabel*)gtk_builder_get_object(builder,"window-root-adaptater-state-state"),
                    .waitIcon = (GtkSpinner*)gtk_builder_get_object(builder,"window-root-adaptater-state-spinner"),
                    .more = {
                        .protocole = (GtkLabel*)gtk_builder_get_object(builder,"window-root-adaptater-state-more-protocol"),
                        .interface = (GtkLabel*)gtk_builder_get_object(builder,"window-root-adaptater-state-more-interface"),
                        .container = (GtkBox*)gtk_builder_get_object(builder,"window-root-adaptater-state-more")
                    }
                }
            }
        };
        mainGui = (MainGui*)malloc(sizeof(MainGui));
        (*mainGui) = gui;

        g_signal_connect(G_OBJECT(mainGui->window),"delete-event",G_CALLBACK(main_onclose),NULL);
        gtk_builder_add_callback_symbol(builder,"window-root-quit",&module_shutdown_main);

        module_init_documentation(builder);        
        module_init_read_codes(builder);
        module_init_options(builder);
        module_init_serial();
        module_init_obd();
        module_init_command_line(builder);
        module_init_vehicle_explorer(builder);

        gtk_builder_connect_signals (builder, NULL);
        g_object_unref (G_OBJECT (builder));

        main_usb_adaptater_state_spinner_wait_for(&module_init_main_deferred);
        
        
    } else {
    
    }
}

int main (int argc, char *argv[])
{
    debug_show_config();
    log_set_from_env();
    log_msg("Starting application ...", LOG_INFO);
    gtk_init (&argc, &argv);
    module_init_main();

    gtk_widget_show (mainGui->window);
    log_msg("Application started", LOG_INFO);
    gtk_main();

    log_msg("Application exit", LOG_INFO);
    return 0;
}
