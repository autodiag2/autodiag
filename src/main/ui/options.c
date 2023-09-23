#include "ui/options.h"
#include "ui/main.h"
#include <stdlib.h>

OptionsGui *optionsGui = null;

void options_hide_window() {
   gtk_widget_hide(optionsGui->window);
}

void options_cancel() {
   module_debug(MODULE_OPTIONS "Cancel options setup");
   options_hide_window();
}

int ensure_serial_in_list(final Serial * port) {
    if ( port == null ) {
        return 1;
    }
    for(int i = 0;i < serial_list_size; i++) {
        if ( serial_list[i] == port ) {
            return 1;
        }
    }
    return 0;
}

void* options_save_internal(void *arg) {
    module_debug(MODULE_OPTIONS "Save options setup");
    serial_close_selected();
    final char *selected_port_name = gtk_combo_box_text_get_active_text(optionsGui->serialList);
    char * activeBaudRateText = gtk_combo_box_text_get_active_text(optionsGui->baudRateSelection);
    final int serial_baud_rate_code = serial_baud_rate_text_to_code(activeBaudRateText);
    g_free(activeBaudRateText);
    if ( selected_port_name == null ) {
        if ( config.com.serial.port_name != null ) {
            free(config.com.serial.port_name);
            config.com.serial.port_name = null;
        }
    } else {
        module_debug(MODULE_OPTIONS "Serial port selected:");
        module_debug(MODULE_OPTIONS (char *)selected_port_name);
        config.com.serial.port_name = strdup(selected_port_name);
        g_free(selected_port_name);
    }
    config.main.adaptater_detailled_settings_showned = gtk_toggle_button_get_active(optionsGui->mainGui.advancedLinkDetails);
    config.commandLine.autoScrollEnabled = gtk_toggle_button_get_active(optionsGui->commandLineGui.outputAutoScroll);
    config_commandLine_showTimestamp_set(gtk_toggle_button_get_active(optionsGui->commandLineGui.showTimestamp));
    config.com.serial.baud_rate = serial_baud_rate_code;
    config.vehicleExplorer.refreshRateS = strtod(gtk_entry_get_text(optionsGui->vehicleExplorerGui.refreshRateS), null);
    config_store();
    config_onchange();
    refresh_usb_adaptater_state_internal(null);
}

void options_save() {
    main_usb_adaptater_state_spinner_wait_for(&options_save_internal);
    options_hide_window();
}

void options_serial_list_refresh() {
    serial_list_fill();
    if ( optionsGui->serialList == null ) {
        module_debug(MODULE_OPTIONS "Cannot process with gui attached");
    } else {
        gtk_combo_box_text_remove_all(optionsGui->serialList);
        for(int serial_i = 0; serial_i < serial_list_size; serial_i++) {
            final SERIAL port = serial_list[serial_i];
            gtk_combo_box_text_append(optionsGui->serialList,NULL,port->name);
            if ( port == serial_list_get_selected() ) {
                gtk_combo_box_set_active((GtkComboBox *)optionsGui->serialList,serial_i);
            }
        }
    }
}

void options_serial_baud_rates_refresh() {
    module_debug(MODULE_OPTIONS "Add some baud rate speed");
    gtk_combo_box_text_remove_all(optionsGui->baudRateSelection);
    gtk_combo_box_text_append(optionsGui->baudRateSelection, NULL, "9600");
    gtk_combo_box_text_append(optionsGui->baudRateSelection, NULL, "38400");
    gtk_combo_box_text_append(optionsGui->baudRateSelection, NULL, "115200");
    gtk_combo_box_text_append(optionsGui->baudRateSelection, NULL, "230400");

    switch(config.com.serial.baud_rate) {
        case BAUD_RATE_9600: gtk_combo_box_set_active((GtkComboBox *)optionsGui->baudRateSelection, 0); break;
        case BAUD_RATE_38400: gtk_combo_box_set_active((GtkComboBox *)optionsGui->baudRateSelection, 1); break;
        case BAUD_RATE_115200: gtk_combo_box_set_active((GtkComboBox *)optionsGui->baudRateSelection, 2); break;
        case BAUD_RATE_230400: gtk_combo_box_set_active((GtkComboBox *)optionsGui->baudRateSelection, 3); break;
        default:
            module_debug(MODULE_OPTIONS "Unknown baud rate");
            gtk_combo_box_set_active((GtkComboBox *)optionsGui->baudRateSelection, -1);
            break;
    }
}

gboolean options_onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused) {
    module_debug(MODULE_OPTIONS "Close event received");
    options_cancel();
    return TRUE;
}

void options_show_window() {
    gtk_widget_show_now (optionsGui->window);
    options_serial_list_refresh();
    options_serial_baud_rates_refresh();
    gtk_toggle_button_set_active(optionsGui->mainGui.advancedLinkDetails, config.main.adaptater_detailled_settings_showned);
    gtk_toggle_button_set_active(optionsGui->commandLineGui.outputAutoScroll, config.commandLine.autoScrollEnabled);
    gtk_toggle_button_set_active(optionsGui->commandLineGui.showTimestamp, config.commandLine.showTimestamp);
    if ( config.com.serial.baud_rate != BAUD_RATE_UNDEFINED ) {
        int index = -1;
        GList * list = gtk_container_get_children((GtkContainer *)optionsGui->baudRateSelection);
        while ( list != null ) {
            index ++;
            if ( serial_baud_rate_text_to_code((char*)list->data) == config.com.serial.baud_rate ) {
                break;
            }
            list = list->next;
        }
        if ( list != null ) {
            gtk_combo_box_set_active((GtkComboBox *)optionsGui->baudRateSelection,index);
        }
    }
    if ( config.com.serial.port_name != null ) {
        int index = -1;
        GList * list = gtk_container_get_children((GtkContainer *)optionsGui->serialList);
        while ( list != null ) {
            index ++;
            if ( strcmp(config.com.serial.port_name,(char*)list->data) == 0 ) {
                break;
            }
            list = list->next;
        }
        if ( list != null ) {
            gtk_combo_box_set_active((GtkComboBox *)optionsGui->serialList,index);
        }
    }
    char * text;
    asprintf(&text,"%f", config.vehicleExplorer.refreshRateS);
    gtk_entry_set_text(optionsGui->vehicleExplorerGui.refreshRateS, text);
    free(text);
}

void module_init_options(GtkBuilder *builder) {
    if ( optionsGui == null ) {
        optionsGui = (OptionsGui*)malloc(sizeof(OptionsGui));
        OptionsGui g = {
            .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-options")),
            .serialList = (GtkComboBoxText*) (gtk_builder_get_object (builder, "window-options-serial-list")),
            .baudRateSelection = (GtkComboBoxText*) (gtk_builder_get_object (builder, "window-options-baud-rate-selection")),
            .mainGui = {
                .advancedLinkDetails = (GtkToggleButton*)gtk_builder_get_object(builder,"window-options-gui-show-advanced-link-details")
            },
            .commandLineGui = {
                .outputAutoScroll = (GtkToggleButton*)gtk_builder_get_object(builder,"window-options-commandLine-autoscrollOnOutput"),
                .showTimestamp = (GtkToggleButton*)gtk_builder_get_object(builder,"window-options-commandLine-showTimestamp")
            },
            .vehicleExplorerGui = {
                .refreshRateS = (GtkEntry*)gtk_builder_get_object(builder,"window-options-refresh-rate-s")
            }
        };
        *optionsGui = g;

        g_signal_connect(G_OBJECT(optionsGui->window),"delete-event",G_CALLBACK(options_onclose),NULL);
        gtk_builder_add_callback_symbol(builder,"window-options-cancel",&options_cancel);
        gtk_builder_add_callback_symbol(builder,"window-options-save",&options_save);
        gtk_builder_add_callback_symbol(builder,"show-window-options",&options_show_window);
        gtk_builder_add_callback_symbol(builder,"window-options-serial-list-refresh-click",&options_serial_list_refresh);
    } else {
    
    }
}

void module_shutdown_options() {
    if ( optionsGui != null ) {
        free(optionsGui);
    }   
}

