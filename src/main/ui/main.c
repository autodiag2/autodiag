#include "ui/main.h"

MainGui *mainGui = null;
static list_mod_gui * mods = null;
static int mod_launcher_last_cols = -1;

void module_shutdown_main() {
    serial_table_close_selected(config.ephemere.device_table);
    serial_table_free(config.ephemere.device_table);
    for(int i = 0; i < mods->size; i++) {
        mods->list[i]->end();
    }
    gtk_main_quit();
}

gboolean main_onclose(final GtkWidget *dialog, final GdkEvent *event, gpointer unused) {
    module_debug(MODULE_MAIN "Close event received");
    module_shutdown_main();
    return true;
}

void debug_show_config() {
    log_msg(LOG_INFO, "=== Running configuration ===");
    #if defined OS_WINDOWS
        log_msg(LOG_INFO, "WINDOWS OS");
    #elif defined OS_POSIX
        log_msg(LOG_INFO, "POSIX OS (using termios)");
    #elif defined OS_UNIX
        log_msg(LOG_INFO, "UNIX OS");
    #else
    #   warning Unsupported OS
    #endif
    log_msg(LOG_INFO, "=============================");
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
    gtk_label_set_markup (GTK_LABEL (mainGui->adaptater.state.more.interface_name), markup);
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
    final Serial * port = serial_table_get_selected(config.ephemere.device_table);
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
        final VehicleIFace* iface = config.ephemere.iface;
        if ( iface->state == VIFaceState_READY ) {
            viface_lock(iface);
            final char * response = elm_print_id((Serial *)iface->device);
            viface_unlock(iface);
            adaptater_interface_set_text((char*)response);
            adaptater_protocol_set_text((char*)iface->device->describe_communication_layer(iface->device));
            free(response);
            gtk_widget_printf(GTK_WIDGET(mainGui->vehicle.manufacturer), "%s", iface->vehicle->manufacturer);
            gtk_widget_printf(GTK_WIDGET(mainGui->vehicle.country), "%s", iface->vehicle->country);
            gtk_widget_printf(GTK_WIDGET(mainGui->vehicle.year), "%d", iface->vehicle->year);
            gtk_widget_printf(GTK_WIDGET(mainGui->vehicle.vin), "%s", buffer_to_ascii(iface->vehicle->vin));
        } else {
            adaptater_protocol_set_text(port->describe_communication_layer(AD_DEVICE(port)));
            adaptater_interface_set_text("Not an OBD interface");
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
    if ( config.com.connectAtStartup ) {
        config_onchange();
    }
    refresh_usb_adaptater_state_internal(null);
    return null;
}
static GtkWidget *grid_child_at(GtkGrid *grid, int col, int row) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(grid));
    for (GList *l = children; l; l = l->next) {
        GtkWidget *w = GTK_WIDGET(l->data);
        int left = 0, top = 0, width = 0, height = 0;
        gtk_container_child_get(GTK_CONTAINER(grid), w,
                                "left-attach", &left,
                                "top-attach", &top,
                                "width", &width,
                                "height", &height,
                                NULL);
        if (left <= col && col < left + width && top <= row && row < top + height) {
            g_list_free(children);
            return w;
        }
    }
    g_list_free(children);
    return NULL;
}
static void grid_next_free_cell(GtkGrid *grid, int cols, int *io_col, int *io_row) {
    int col = *io_col;
    int row = *io_row;
    for (;;) {
        if (cols < 1) cols = 1;
        if (col >= cols) { col = 0; row++; }

        if (grid_child_at(grid, col, row) == NULL) {
            *io_col = col;
            *io_row = row;
            return;
        }
        col++;
    }
}
static int grid_compute_max_cols(GtkWidget *grid, int fallback_cols, int item_min_w, int spacing) {
    int w = gtk_widget_get_allocated_width(grid);
    if (w <= 1) return (fallback_cols < 1) ? 1 : fallback_cols;

    int cols = (w + spacing) / (item_min_w + spacing);
    if (cols < 1) cols = 1;
    return cols;
}

static void grid_clear_children_tagged(GtkWidget *grid) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(grid));
    for (GList *l = children; l; l = l->next) {
        GtkWidget *w = GTK_WIDGET(l->data);
        if (g_object_get_data(G_OBJECT(w), "autolayout_dynamic") != NULL) {
            g_object_ref(w);
            gtk_container_remove(GTK_CONTAINER(grid), w);
        }
    }
    g_list_free(children);
}
static void populate_mod_launcher_grid(GtkWidget *grid, list_mod_gui *mods) {
    const int fallback_cols = 3;
    const int item_min_w = 160;
    const int spacing = 10;

    gtk_grid_set_column_spacing(GTK_GRID(grid), spacing);
    gtk_grid_set_row_spacing(GTK_GRID(grid), spacing);
    gtk_widget_set_margin_start(grid, 10);
    gtk_widget_set_margin_end(grid, 10);
    gtk_widget_set_margin_top(grid, 10);
    gtk_widget_set_margin_bottom(grid, 10);

    int cols = grid_compute_max_cols(grid, fallback_cols, item_min_w, spacing);
    if (cols < 1) cols = 1;

    grid_clear_children_tagged(grid);

    int i = 0;
    for (int mi = 0; mi < mods->size; mi++) {
        mod_gui *m = mods->list[mi];

        int col = i % cols;
        int row = i / cols;
        grid_next_free_cell(GTK_GRID(grid), cols, &col, &row);

        GtkWidget *launcher = gtk_button_new_with_label(m->name);
        g_object_set_data(G_OBJECT(launcher), "autolayout_dynamic", (gpointer)1);

        gtk_widget_set_size_request(launcher, item_min_w, -1);
        gtk_widget_set_hexpand(launcher, TRUE);
        gtk_widget_set_halign(launcher, GTK_ALIGN_FILL);
        
        gtk_grid_attach(GTK_GRID(grid), launcher, col, row, 1, 1);
        g_signal_connect(G_OBJECT(launcher), "clicked", G_CALLBACK(m->show), NULL);
        gtk_widget_show(launcher);

        i++;
    }

    gtk_widget_show_all(grid);
    gtk_widget_queue_resize(grid);
}

static void mod_launcher_on_size_allocate(GtkWidget *grid, GdkRectangle *alloc, gpointer user_data) {
    list_mod_gui *mods = (list_mod_gui*)user_data;

    const int fallback_cols = 3;
    const int item_min_w = 160;
    const int spacing = 10;

    int cols = grid_compute_max_cols(grid, fallback_cols, item_min_w, spacing);
    if (cols < 1) cols = 1;

    if (cols == mod_launcher_last_cols) return;
    mod_launcher_last_cols = cols;

    populate_mod_launcher_grid(grid, mods);
}

void module_init_main() {
    if ( mainGui == null ) {

        char * ui_dir = installation_folder_resolve("ui"), *mainUiPath, *mainCSSPath;
        if ( ui_dir == null ) {
            log_msg(LOG_ERROR, "Data directory not found try to reinstall the software");
            exit(1);
        }
        asprintf(&mainUiPath, "%s"  PATH_FOLDER_DELIM "main.glade", ui_dir);
        asprintf(&mainCSSPath, "%s" PATH_FOLDER_DELIM "main.css",   ui_dir);
        final GtkBuilder *builder = gtk_builder_new_from_file(mainUiPath);
        free(ui_dir);
        free(mainUiPath);

        final MainGui gui = {
            .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-root")),
            .adaptater = {
                .state = {
                    .text = (GtkLabel*)gtk_builder_get_object(builder,"window-root-adaptater-state-state"),
                    .waitIcon = (GtkSpinner*)gtk_builder_get_object(builder,"window-root-adaptater-state-spinner"),
                    .more = {
                        .protocole = (GtkLabel*)gtk_builder_get_object(builder,"window-root-adaptater-state-more-protocol"),
                        .interface_name = (GtkLabel*)gtk_builder_get_object(builder,"window-root-adaptater-state-more-interface"),
                        .container = (GtkBox*)gtk_builder_get_object(builder,"window-root-adaptater-state-more")
                    }
                }
            },
            .vehicle = {
                .manufacturer = GTK_LABEL(gtk_builder_get_object(builder, "window-root-vehicle-manufacturer")),
                .country = GTK_LABEL(gtk_builder_get_object(builder, "window-root-vehicle-country")),
                .year = GTK_LABEL(gtk_builder_get_object(builder, "window-root-vehicle-year")),
                .vin = GTK_LABEL(gtk_builder_get_object(builder, "window-root-vehicle-vin"))
            },
            .mod_launcher = GTK_GRID(gtk_builder_get_object(builder, "root-actions"))
        };
                            
        mainGui = (MainGui*)malloc(sizeof(MainGui));
        (*mainGui) = gui;
        gtk_wrapper_set_window_icon(GTK_WINDOW(gui.window));

        {
            GError * error = null;
            GtkCssProvider * css_provider = gtk_css_provider_new();
            if ( ! gtk_css_provider_load_from_path (css_provider, mainCSSPath, &error) ) {
                log_msg(LOG_WARNING, "Error while loading the css '%s': %s (domain:%d/%s code:%d)", mainCSSPath,error->message,error->domain,g_quark_to_string(error->domain),error->code);
            }  
            free(mainCSSPath);
            
            GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(gui.window));
            gtk_style_context_add_provider_for_screen(screen,
                                    GTK_STYLE_PROVIDER(css_provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_USER);
        }

        assert(0 != g_signal_connect(G_OBJECT(mainGui->window),"delete-event",G_CALLBACK(main_onclose),NULL));
        gtk_builder_add_callback_symbol(builder,"window-root-quit",&module_shutdown_main);

        serial_table_fill(config.ephemere.device_table);
        
        mods = list_mod_gui_new();
        list_mod_gui_append(mods, mod_gui_dyno_new());
        list_mod_gui_append(mods, mod_gui_vehicle_explorer_new());
        list_mod_gui_append(mods, mod_gui_options_new());
        list_mod_gui_append(mods, mod_gui_command_line_new());
        list_mod_gui_append(mods, mod_gui_read_codes_new());
        list_mod_gui_append(mods, mod_gui_documentation_new());

        list_mod_gui_build(mods, builder);
        
        g_signal_connect(G_OBJECT(mainGui->mod_launcher), "size-allocate",
                     G_CALLBACK(mod_launcher_on_size_allocate), mods);

        populate_mod_launcher_grid(GTK_WIDGET(mainGui->mod_launcher), mods);

        gtk_builder_connect_signals (builder, NULL);
        g_object_unref (G_OBJECT (builder));

        main_usb_adaptater_state_spinner_wait_for(&module_init_main_deferred);
        
        
    } else {
    
    }
}
