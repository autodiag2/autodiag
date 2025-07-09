#include "ui/options.h"
#include "ui/main.h"

static OptionsGui *gui = null;

static void simutation_add_ecu(char *address, char *generator) {
    GtkBox * container = gui->simulator.ecus.container;
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    int index = get_container_child_count(GTK_CONTAINER(container));
    gtk_style_context_add_class(gtk_widget_get_style_context(row),
        (index % 2 == 0) ? "row-grey" : "row-grey-dark"
    );
    gtk_widget_set_margin_top(row, 5);
    gtk_widget_set_margin_bottom(row, 5);
    gtk_widget_set_margin_start(row, 5);
    gtk_widget_set_margin_end(row, 5);

    char addr_text[16];
    snprintf(addr_text, sizeof(addr_text), "%02X", (unsigned char)strtoul(address, NULL, 16));
    GtkWidget *label_addr = gtk_label_new(addr_text);
    gtk_widget_set_halign(label_addr, GTK_ALIGN_START);

    GtkWidget *combo_gen = gtk_combo_box_text_new();
    g_signal_connect(combo_gen, "scroll-event", G_CALLBACK(gtk_combo_box_text_prevent_scroll), NULL);
    char generators[][50] = {"random", "cycle", "citroen_c5_x7", "gui"};
    int generators_len = 4;
    for(int i = 0; i < generators_len; i ++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_gen), generators[i]);
        if ( strcmp(generator, generators[i]) == 0 ) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo_gen), i);
        }
    }

    GtkWidget *del_button = gtk_button_new_with_label("Delete");
    g_signal_connect_swapped(del_button, "clicked", G_CALLBACK(gtk_widget_destroy), row);

    gtk_box_pack_start(GTK_BOX(row), label_addr, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(row), combo_gen, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(row), del_button, FALSE, FALSE, 0);

    gtk_box_pack_start(container, row, FALSE, FALSE, 2);
    gtk_widget_show_all(row);
}

static void simutation_add_clicked(GtkButton *button, gpointer user_data) {
    const char *addr_text = gtk_entry_get_text(gui->simulator.ecus.address);
    char *gen_text = gtk_combo_box_text_get_active_text(gui->simulator.ecus.generator);
    if (gen_text == NULL) {
        gen_text = strdup("random");
    }

    if (addr_text == NULL || strcmp(addr_text, "") == 0) {
        GtkBox *container = gui->simulator.ecus.container;
        GList *children = gtk_container_get_children(GTK_CONTAINER(container));
        GtkWidget *last = g_list_last(children) ? g_list_last(children)->data : NULL;
        char new_addr[16] = "E8";
        if (last) {
            GList *last_row_children = gtk_container_get_children(GTK_CONTAINER(last));
            GtkWidget *label = last_row_children ? last_row_children->data : NULL;
            if (GTK_IS_LABEL(label)) {
                const char *label_text = gtk_label_get_text(GTK_LABEL(label));
                unsigned long val = strtoul(label_text, NULL, 16);
                snprintf(new_addr, sizeof(new_addr), "%02lX", (val + 1) & 0xFF);
            }
            g_list_free(last_row_children);
        }
        g_list_free(children);
        addr_text = strdup(new_addr);
    }

    simutation_add_ecu((char *)addr_text, gen_text);
    free(gen_text);
}

static void hide_window() {
   gtk_widget_hide(gui->window);
}

static void cancel() {
   module_debug(MODULE_OPTIONS "Cancel options setup");
   hide_window();
}

static void serial_list_changed(GtkComboBoxText *combo, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(user_data);
    gchar *text = gtk_combo_box_text_get_active_text(combo);
    gtk_entry_set_text(entry, text ? text : "");
}
static void* save_internal(void *arg) {
    module_debug(MODULE_OPTIONS "Save options setup");
    serial_close_selected();
    final char * device_location = gtk_entry_get_text(gui->device_location);
    if ( device_location == null || strcmp(device_location,"") == 0 ) {
        if ( config.com.serial.device_location != null ) {
            free(config.com.serial.device_location);
            config.com.serial.device_location = null;
        }
    } else {
        module_debug(MODULE_OPTIONS "Serial port selected:");
        module_debug(MODULE_OPTIONS (char *)device_location);
        config.com.serial.device_location = strdup(device_location);
    }
    const char * activeBaudRateText = gtk_entry_get_text(gui->baudRateSelection);
    int baud_rate = atoi(activeBaudRateText);
    if ( baud_rate < 0 ) {
        log_msg(LOG_ERROR, "Invalid baud rate setup '%s'", activeBaudRateText);
    } else {
        config.com.serial.baud_rate = baud_rate;
    }
    config.log.level = log_level_from_str(gtk_combo_box_text_get_active_text(gui->logLevel));
    if ( ! log_is_env_set() ) {
        log_set_level(config.log.level);
    }
    config.main.adaptater_detailled_settings_showned = gtk_toggle_button_get_active(gui->mainGui.advancedLinkDetails);
    config.commandLine.autoScrollEnabled = gtk_toggle_button_get_active(gui->commandLineGui.outputAutoScroll);
    config_commandLine_showTimestamp_set(gtk_toggle_button_get_active(gui->commandLineGui.showTimestamp));
    config.vehicleExplorer.refreshRateS = strtod(gtk_entry_get_text(gui->vehicleExplorerGui.refreshRateS), null);
    config.vehicleInfos.vin = strdup(gtk_entry_get_text(gui->vehicleInfos.vin));
    config_store();
    config_onchange();
    final VehicleIFace * iface = config.ephemere.iface;
    if ( iface != null ) {
        viface_fill_infos_from_vin(iface);
        final char * manufacturer = gtk_combo_box_text_get_active_text(gui->vehicleInfos.manufacturer);
        if ( manufacturer != null ) {
            if ( iface->vehicle->manufacturer != null ) {
                free(iface->vehicle->manufacturer);
            }
            iface->vehicle->manufacturer = manufacturer;
        }
        final char * engine = gtk_combo_box_text_get_active_text(gui->vehicleInfos.engine);
        if ( engine != null ) {
            if ( iface->vehicle->engine != null ) {
                free(iface->vehicle->engine);
            }
            iface->vehicle->engine = engine;
        }
        iface->vehicle->engine = engine;
    }
    refresh_usb_adaptater_state_internal(null);
    return null;
}

static void save() {
    main_usb_adaptater_state_spinner_wait_for(&save_internal);
    hide_window();
}

static void list_serial_refresh() {
    list_serial_fill();
    if ( gui->serialList == null ) {
        module_debug(MODULE_OPTIONS "Cannot process without gui attached");
    } else {
        gtk_combo_box_text_remove_all(gui->serialList);
        for(int serial_i = 0; serial_i < list_serial.size; serial_i++) {
            final Serial * port = list_serial.list[serial_i];
            gtk_combo_box_text_append(gui->serialList,NULL,port->location);
            if ( port == list_serial_get_selected() ) {
                gtk_combo_box_set_active((GtkComboBox *)gui->serialList,serial_i);
                gtk_entry_set_text(gui->device_location, port->location);
            }
        }
    }
}
static void fill_vehicle_infos() {
    gtk_combo_box_text_remove_all(gui->vehicleInfos.manufacturer);
    gtk_combo_box_text_remove_all(gui->vehicleInfos.engine);

    gtk_combo_box_text_append(gui->vehicleInfos.manufacturer, NULL, "");
    gtk_combo_box_text_append(gui->vehicleInfos.engine, NULL, "");

    GHashTable *manufacturers = g_hash_table_new(g_str_hash, g_str_equal);
    GHashTable *engines = g_hash_table_new(g_str_hash, g_str_equal);

    VehicleIFace *iface = config.ephemere.iface;
    int manufacturer_i = 1, engine_i = 1;
    int manufacturer_active_i = -1, engine_active_i = -1;

    for (int vehicle_i = 0; vehicle_i < database.size; vehicle_i++) {
        final Vehicle *vehicle = database.list[vehicle_i];

        if (vehicle->manufacturer != null && !g_hash_table_contains(manufacturers, vehicle->manufacturer)) {
            g_hash_table_add(manufacturers, vehicle->manufacturer);
            gtk_combo_box_text_append(gui->vehicleInfos.manufacturer, NULL, vehicle->manufacturer);
            if (iface != null && iface->vehicle->manufacturer != null) {
                if (strcmp(iface->vehicle->manufacturer, vehicle->manufacturer) == 0) {
                    manufacturer_active_i = manufacturer_i;
                }
            }
            manufacturer_i++;
        }

        if (vehicle->engine != null && !g_hash_table_contains(engines, vehicle->engine)) {
            g_hash_table_add(engines, vehicle->engine);
            gtk_combo_box_text_append(gui->vehicleInfos.engine, NULL, vehicle->engine);
            if (iface != null && iface->vehicle->engine != null) {
                if (strcmp(iface->vehicle->engine, vehicle->engine) == 0) {
                    engine_active_i = engine_i;
                }
            }
            engine_i++;
        }
    }
    if ( manufacturer_active_i != -1 ) {
        gtk_combo_box_set_active((GtkComboBox *)gui->vehicleInfos.manufacturer, manufacturer_active_i);
    }
    if ( engine_active_i != -1 ) {
        gtk_combo_box_set_active((GtkComboBox *)gui->vehicleInfos.engine, engine_active_i);
    }
    g_hash_table_destroy(manufacturers);
    g_hash_table_destroy(engines);

    gtk_entry_set_text(gui->vehicleInfos.vin, config.vehicleInfos.vin == null ? "" : config.vehicleInfos.vin);
}
static gboolean onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused) {
    module_debug(MODULE_OPTIONS "Close event received");
    cancel();
    return TRUE;
}
static void set_device_location(char * location) {
    gtk_entry_set_text(gui->device_location, location);
    for(int serial_i = 0; serial_i < list_serial.size; serial_i++) {
        final Serial * port = list_serial.list[serial_i];
        if ( strcmp(location, port->location) == 0 ) {
            gtk_combo_box_set_active((GtkComboBox *)gui->serialList,serial_i);
            break;
        }
    }
}

static void show_window() {
    gtk_widget_show_now (gui->window);
    list_serial_refresh();
    fill_vehicle_infos();
    gtk_toggle_button_set_active(gui->mainGui.advancedLinkDetails, config.main.adaptater_detailled_settings_showned);
    gtk_toggle_button_set_active(gui->commandLineGui.outputAutoScroll, config.commandLine.autoScrollEnabled);
    gtk_toggle_button_set_active(gui->commandLineGui.showTimestamp, config.commandLine.showTimestamp);
    gtk_combo_box_set_active(GTK_COMBO_BOX(gui->logLevel), config.log.level);
    char * txt;
    asprintf(&txt,"%d", config.com.serial.baud_rate);
    gtk_entry_set_text(gui->baudRateSelection,txt);
    free(txt);
    if ( config.com.serial.device_location != null ) {
        set_device_location(config.com.serial.device_location);
    }
    char * text;
    asprintf(&text,"%f", config.vehicleExplorer.refreshRateS);
    gtk_entry_set_text(gui->vehicleExplorerGui.refreshRateS, text);
    free(text);

    GList *children = gtk_container_get_children(GTK_CONTAINER(gui->simulator.ecus.container));
    for (GList *iter = children; iter != NULL; iter = iter->next) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    simutation_add_ecu("E8", "random");
}

void window_baud_rate_set_from_button(final GtkButton * button) {
    char* baud_rate_str = (char*)gtk_button_get_label(button);
    gtk_entry_set_text(gui->baudRateSelection, baud_rate_str);
}

static gboolean launch_simulation_set_pending_text(gpointer data) {
    SimELM327 * elm327 = (SimELM327*)data;
    gtk_label_set_text(gui->simulator.launchDesc, "Starting simulation ...");
    return false;
}
static gboolean launch_simulation_update_gui(gpointer data) {
    SimELM327 * elm327 = (SimELM327*)data;

    list_serial_refresh();

    char * fmt = elm327->device_location == null ? 
            "Simulation not started ..." 
            : "Simulation started at '%s'";
    char * simu_desc;
    asprintf(&simu_desc,fmt,elm327->device_location);
    gtk_label_set_text(gui->simulator.launchDesc, simu_desc);
    free(simu_desc);
    
    set_device_location(elm327->device_location);

    return false;
}

static void launch_simulation_internal() {
    gtk_spinner_start(gui->simulator.spinner);

    SimELM327 *elm327 = sim_elm327_new();
    bool firstPass = true;    

    GList *rows = gtk_container_get_children(GTK_CONTAINER(gui->simulator.ecus.container));

    for (GList *iter = rows; iter != NULL; iter = iter->next) {
        if ( firstPass ) {
            list_SimECU_empty(elm327->ecus);
            firstPass = false;
        }
        GtkWidget *row = GTK_WIDGET(iter->data);
        // children order: [0]=label_addr, [1]=combo_gen, [2]=del_button
        GList *children = gtk_container_get_children(GTK_CONTAINER(row));

        GtkWidget *label_addr = GTK_WIDGET(g_list_nth_data(children, 0));
        const char *addr_text = gtk_label_get_text(GTK_LABEL(label_addr));
        unsigned int address = 0;
        if ( sscanf(addr_text, "%x", &address) != 1 ) {
            log_msg(LOG_ERROR, "should raise a popup telling incorrect address specified");
            address = 0xE8;
        }
        SimECU *ecu = sim_ecu_emulation_new(address);

        GtkWidget *combo_gen = GTK_WIDGET(g_list_nth_data(children, 1));

        const char *type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_gen));
        if (!type) type = "random";

        if (strcasecmp(type, "random") == 0) {
            ecu->generator = sim_ecu_generator_new_random();
        } else if (strcasecmp(type, "cycle") == 0) {
            ecu->generator = sim_ecu_generator_new_cycle();
        } else if (strcasecmp(type, "citroen_c5_x7") == 0) {
            ecu->generator = sim_ecu_generator_new_citroen_c5_x7();
        } else if (strcasecmp(type, "gui") == 0) {
            ecu->generator = sim_ecu_generator_new_gui();
            char address[3];
            sprintf(address, "%02hhX", ecu->address);
            sim_ecu_generator_gui_set_context(ecu->generator, address);
            g_idle_add(sim_ecu_generator_gui_show_gsource, ecu->generator->context);
        } else {
            log_msg(LOG_ERROR, "Unknown generator type: '%s'", type);
        }

        g_list_free(children);
        list_SimECU_append(elm327->ecus,ecu);
    }
    g_list_free(rows);

    sim_elm327_loop_as_daemon(elm327);
    g_idle_add(launch_simulation_set_pending_text, elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    g_idle_add(launch_simulation_update_gui, elm327);
}

static static void launch_simulation_clean_up_routine(void *arg) {
    obd_thread_cleanup_routine(arg);
    gtk_spinner_stop(gui->simulator.spinner);
}

THREAD_WRITE_DAEMON(
        launch_simulation_daemon,
        launch_simulation_internal,
        launch_simulation_clean_up_routine, gui->simulator.launchThread
)


static void launch_simulation() {
    thread_allocate_and_start(&gui->simulator.launchThread,&launch_simulation_daemon);
}

void module_init_options(GtkBuilder *builder) {
    if ( gui == null ) {
        gui = (OptionsGui*)malloc(sizeof(OptionsGui));
        OptionsGui g = {
            .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-options")),
            .logLevel = GTK_COMBO_BOX_TEXT(gtk_builder_get_object (builder, "window-options-log-level")),
            .serialList = GTK_COMBO_BOX_TEXT(gtk_builder_get_object (builder, "window-options-serial-list")),
            .device_location = GTK_ENTRY(gtk_builder_get_object(builder, "options-device-location")),
            .baudRateSelection = GTK_ENTRY(gtk_builder_get_object (builder, "window-options-baud-rate-selection")),
            .mainGui = {
                .advancedLinkDetails = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,"window-options-gui-show-advanced-link-details"))
            },
            .commandLineGui = {
                .outputAutoScroll = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,"window-options-commandLine-autoscrollOnOutput")),
                .showTimestamp = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,"window-options-commandLine-showTimestamp"))
            },
            .vehicleExplorerGui = {
                .refreshRateS = GTK_ENTRY(gtk_builder_get_object(builder,"window-options-refresh-rate-s"))
            },
            .simulator = {
                .spinner = GTK_SPINNER(gtk_builder_get_object(builder,"window-simulation-spinner")),
                .launchDesc = GTK_LABEL(gtk_builder_get_object(builder,"window-simulation-launch-description")),
                .launchThread = null,
                .ecus = {
                    .container = GTK_BOX(gtk_builder_get_object(builder, "options-simulation-ecus-container")),
                    .add = GTK_BUTTON(gtk_builder_get_object(builder, "options-simulation-ecus-add")),
                    .address = GTK_ENTRY(gtk_builder_get_object(builder, "options-simulation-ecus-address")),
                    .generator = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "options-simulation-ecus-generator")),
                }
            },
            .vehicleInfos = {
                .manufacturer = GTK_COMBO_BOX_TEXT(gtk_builder_get_object (builder, "window-options-vehicle-manufacturer")),
                .engine = GTK_COMBO_BOX_TEXT(gtk_builder_get_object (builder, "window-options-vehicle-engine")),
                .vin = GTK_ENTRY(gtk_builder_get_object (builder, "window-options-vehicle-vin"))
            }
        };
        *gui = g;

        db_vehicle_load_in_memory();
        g_signal_connect(g.serialList, "changed", G_CALLBACK(serial_list_changed), gui->device_location);
        g_signal_connect(g.logLevel, "scroll-event", G_CALLBACK(gtk_combo_box_text_prevent_scroll), NULL);
        g_signal_connect(g.serialList, "scroll-event", G_CALLBACK(gtk_combo_box_text_prevent_scroll), NULL);
        g_signal_connect(g.simulator.ecus.generator, "scroll-event", G_CALLBACK(gtk_combo_box_text_prevent_scroll), NULL);
        g_signal_connect(g.vehicleInfos.engine, "scroll-event", G_CALLBACK(gtk_combo_box_text_prevent_scroll), NULL);
        g_signal_connect(g.vehicleInfos.manufacturer, "scroll-event", G_CALLBACK(gtk_combo_box_text_prevent_scroll), NULL);
        g_signal_connect(g.simulator.ecus.add, "clicked", G_CALLBACK(simutation_add_clicked), builder);
        gtk_builder_add_callback_symbol(builder,"window-options-baud-rate-set-from-button",G_CALLBACK(&window_baud_rate_set_from_button));
        g_signal_connect(G_OBJECT(gui->window),"delete-event",G_CALLBACK(onclose),NULL);
        gtk_builder_add_callback_symbol(builder,"window-simulation-launch-clicked",&launch_simulation);
        gtk_builder_add_callback_symbol(builder,"window-options-cancel",&cancel);
        gtk_builder_add_callback_symbol(builder,"window-options-save",&save);
        gtk_builder_add_callback_symbol(builder,"show-window-options",&show_window);
        gtk_builder_add_callback_symbol(builder,"window-options-serial-list-refresh-click",&list_serial_refresh);
    } else {
    
    }
}

void module_shutdown_options() {
    if ( gui != null ) {
        free(gui);
    }   
}

