#include "ui/options.h"
#include "ui/main.h"

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
    for(int i = 0;i < serial_list.size; i++) {
        if ( serial_list.list[i] == port ) {
            return 1;
        }
    }
    return 0;
}

void* options_save_internal(void *arg) {
    module_debug(MODULE_OPTIONS "Save options setup");
    serial_close_selected();
    final char *selected_port_name = gtk_combo_box_text_get_active_text(optionsGui->serialList);
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
    const char * activeBaudRateText = gtk_entry_get_text(optionsGui->baudRateSelection);
    int baud_rate = atoi(activeBaudRateText);
    if ( baud_rate < 0 ) {
        log_msg(LOG_ERROR, "Invalid baud rate setup '%s'", activeBaudRateText);
    } else {
        config.com.serial.baud_rate = baud_rate;
    }
    config.log.level = log_level_from_str(gtk_combo_box_text_get_active_text(optionsGui->logLevel));
    if ( ! log_is_env_set() ) {
        log_set_level(config.log.level);
    }
    config.main.adaptater_detailled_settings_showned = gtk_toggle_button_get_active(optionsGui->mainGui.advancedLinkDetails);
    config.commandLine.autoScrollEnabled = gtk_toggle_button_get_active(optionsGui->commandLineGui.outputAutoScroll);
    config_commandLine_showTimestamp_set(gtk_toggle_button_get_active(optionsGui->commandLineGui.showTimestamp));
    config.vehicleExplorer.refreshRateS = strtod(gtk_entry_get_text(optionsGui->vehicleExplorerGui.refreshRateS), null);
    config_store();
    config_onchange();
    final OBDIFace * iface = config.ephemere.iface;
    final char * brand = gtk_combo_box_text_get_active_text(optionsGui->vehicleInfos.brand);
    if ( iface->vehicle->brand != null ) {
        free(iface->vehicle->brand);
    }
    iface->vehicle->brand = brand;
    final char * engine = gtk_combo_box_text_get_active_text(optionsGui->vehicleInfos.engine);
    if ( iface->vehicle->engine != null ) {
        free(iface->vehicle->engine);
    }
    iface->vehicle->engine = engine;
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
        for(int serial_i = 0; serial_i < serial_list.size; serial_i++) {
            final SERIAL port = serial_list.list[serial_i];
            gtk_combo_box_text_append(optionsGui->serialList,NULL,port->name);
            if ( port == serial_list_get_selected() ) {
                gtk_combo_box_set_active((GtkComboBox *)optionsGui->serialList,serial_i);
            }
        }
    }
}
void options_fill_vehicle_infos() {
    db_vehicle_load_in_memory();
    gtk_combo_box_text_remove_all(optionsGui->vehicleInfos.brand);
    gtk_combo_box_text_remove_all(optionsGui->vehicleInfos.engine);

    GHashTable *brands = g_hash_table_new(g_str_hash, g_str_equal);
    GHashTable *engines = g_hash_table_new(g_str_hash, g_str_equal);

    OBDIFace *iface = config.ephemere.iface;
    int brand_i = 0, engine_i = 0;

    for (int vehicle_i = 0; vehicle_i < database.size; vehicle_i++) {
        final Vehicle *vehicle = database.list[vehicle_i];

        if (vehicle->brand != null && !g_hash_table_contains(brands, vehicle->brand)) {
            g_hash_table_add(brands, vehicle->brand);
            gtk_combo_box_text_append(optionsGui->vehicleInfos.brand, NULL, vehicle->brand);
            if (iface != null && iface->vehicle->brand != null) {
                if (strcmp(iface->vehicle->brand, vehicle->brand) == 0) {
                    gtk_combo_box_set_active((GtkComboBox *)optionsGui->vehicleInfos.brand, brand_i);
                }
            }
            brand_i++;
        }

        if (vehicle->engine != null && !g_hash_table_contains(engines, vehicle->engine)) {
            g_hash_table_add(engines, vehicle->engine);
            gtk_combo_box_text_append(optionsGui->vehicleInfos.engine, NULL, vehicle->engine);
            if (iface != null && iface->vehicle->engine != null) {
                if (strcmp(iface->vehicle->engine, vehicle->engine) == 0) {
                    gtk_combo_box_set_active((GtkComboBox *)optionsGui->vehicleInfos.engine, engine_i);
                }
            }
            engine_i++;
        }
    }

    g_hash_table_destroy(brands);
    g_hash_table_destroy(engines);
}
gboolean options_onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused) {
    module_debug(MODULE_OPTIONS "Close event received");
    options_cancel();
    return TRUE;
}
void options_set_serial_select_from_name(char * name) {
    for(int serial_i = 0; serial_i < serial_list.size; serial_i++) {
        final SERIAL port = serial_list.list[serial_i];
        if ( strcmp(name, port->name) == 0 ) {
            gtk_combo_box_set_active((GtkComboBox *)optionsGui->serialList,serial_i);
            break;
        }
    }
}

void options_show_window() {
    gtk_widget_show_now (optionsGui->window);
    options_serial_list_refresh();
    options_fill_vehicle_infos();
    gtk_toggle_button_set_active(optionsGui->mainGui.advancedLinkDetails, config.main.adaptater_detailled_settings_showned);
    gtk_toggle_button_set_active(optionsGui->commandLineGui.outputAutoScroll, config.commandLine.autoScrollEnabled);
    gtk_toggle_button_set_active(optionsGui->commandLineGui.showTimestamp, config.commandLine.showTimestamp);
    gtk_combo_box_set_active(GTK_COMBO_BOX(optionsGui->logLevel), config.log.level);
    char * txt;
    asprintf(&txt,"%d", config.com.serial.baud_rate);
    gtk_entry_set_text(optionsGui->baudRateSelection,txt);
    free(txt);
    if ( config.com.serial.port_name != null ) {
        options_set_serial_select_from_name(config.com.serial.port_name);
    }
    char * text;
    asprintf(&text,"%f", config.vehicleExplorer.refreshRateS);
    gtk_entry_set_text(optionsGui->vehicleExplorerGui.refreshRateS, text);
    free(text);
}

void window_options_baud_rate_set_from_button(final GtkButton * button) {
    char* baud_rate_str = (char*)gtk_button_get_label(button);
    gtk_entry_set_text(optionsGui->baudRateSelection, baud_rate_str);
}

gboolean options_launch_simulation_set_pending_text(gpointer data) {
    ELM327emulation * elm327 = (ELM327emulation*)data;
    gtk_label_set_text(optionsGui->simulator.launchDesc, "Starting simulation ...");
    return false;
}
gboolean options_launch_simulation_update_gui(gpointer data) {
    ELM327emulation * elm327 = (ELM327emulation*)data;

    options_serial_list_refresh();

    char * fmt = elm327->port_name == null ? 
            "Simulation not started ..." 
            : "Simulation started at '%s'";
    char * simu_desc;
    asprintf(&simu_desc,fmt,elm327->port_name);
    gtk_label_set_text(optionsGui->simulator.launchDesc, simu_desc);
    free(simu_desc);
    
    options_set_serial_select_from_name(elm327->port_name);

    return false;
}

void options_launch_simulation_internal() {
    gtk_spinner_start(optionsGui->simulator.spinner);
    ELM327emulation * elm327 = elm327_sim_new();
    elm327_sim_loop_as_daemon(elm327);
    g_idle_add(options_launch_simulation_set_pending_text, elm327);
    usleep(500e3);
    g_idle_add(options_launch_simulation_update_gui, elm327);
}
static void options_launch_simulation_clean_up_routine(void *arg) {
    obd_thread_cleanup_routine(arg);
    gtk_spinner_stop(optionsGui->simulator.spinner);
}

THREAD_WRITE_DAEMON(
        options_launch_simulation_daemon,
        options_launch_simulation_internal,
        options_launch_simulation_clean_up_routine, optionsGui->simulator.launchThread
)

void options_launch_simulation() {
    thread_allocate_and_start(&optionsGui->simulator.launchThread,&options_launch_simulation_daemon);
}

void module_init_options(GtkBuilder *builder) {
    if ( optionsGui == null ) {
        optionsGui = (OptionsGui*)malloc(sizeof(OptionsGui));
        OptionsGui g = {
            .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-options")),
            .logLevel = (GtkComboBoxText*) (gtk_builder_get_object (builder, "window-options-log-level")),
            .serialList = (GtkComboBoxText*) (gtk_builder_get_object (builder, "window-options-serial-list")),
            .baudRateSelection = GTK_ENTRY(gtk_builder_get_object (builder, "window-options-baud-rate-selection")),
            .mainGui = {
                .advancedLinkDetails = (GtkToggleButton*)gtk_builder_get_object(builder,"window-options-gui-show-advanced-link-details")
            },
            .commandLineGui = {
                .outputAutoScroll = (GtkToggleButton*)gtk_builder_get_object(builder,"window-options-commandLine-autoscrollOnOutput"),
                .showTimestamp = (GtkToggleButton*)gtk_builder_get_object(builder,"window-options-commandLine-showTimestamp")
            },
            .vehicleExplorerGui = {
                .refreshRateS = GTK_ENTRY(gtk_builder_get_object(builder,"window-options-refresh-rate-s"))
            },
            .simulator = {
                .spinner = GTK_SPINNER(gtk_builder_get_object(builder,"window-simulation-spinner")),
                .launchDesc = GTK_LABEL(gtk_builder_get_object(builder,"window-simulation-launch-description")),
                .launchThread = null
            },
            .vehicleInfos = {
                .brand = (GtkComboBoxText*) (gtk_builder_get_object (builder, "window-options-vehicle-brand")),
                .engine = (GtkComboBoxText*) (gtk_builder_get_object (builder, "window-options-vehicle-engine"))
            }
        };
        *optionsGui = g;

        gtk_builder_add_callback_symbol(builder,"window-options-baud-rate-set-from-button",G_CALLBACK(&window_options_baud_rate_set_from_button));
        g_signal_connect(G_OBJECT(optionsGui->window),"delete-event",G_CALLBACK(options_onclose),NULL);
        gtk_builder_add_callback_symbol(builder,"window-simulation-launch-clicked",&options_launch_simulation);
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

