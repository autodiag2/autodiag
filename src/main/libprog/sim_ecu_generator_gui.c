#include "libprog/sim_ecu_generator_gui.h"

static Buffer * response_saej1979_dtcs(SimECUGenerator *generator, int service_id) {
    SimECUGeneratorGui *gui = (SimECUGeneratorGui *)generator->context;
    Buffer * binResponse = ad_buffer_new();
    if ( service_id == OBD_SERVICE_SHOW_DTC ) {
        if ( ! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->dtcs.dtcCleared)) ) {
            GList *ptr = gtk_container_get_children(GTK_CONTAINER(gui->dtcs.listView));
            ad_list_Buffer * dtcs = ad_list_Buffer_new();
            while (ptr != NULL) {
                GList *ptr_next = ptr->next;
                GtkWidget *row = GTK_WIDGET(ptr->data);

                if (GTK_IS_LIST_BOX_ROW(row)) {
                    GtkWidget *child = gtk_bin_get_child(GTK_BIN(row));
                    if (GTK_IS_LABEL(child)) {
                        const char *dtc = gtk_label_get_text(GTK_LABEL(child));
                        Buffer *dtc_bin = saej1979_dtc_bin_from_string((char*)dtc);
                        if ( dtc_bin == null ) {
                            log_msg(LOG_ERROR, "invalid dtc found");
                        } else {
                            ad_list_Buffer_append(dtcs, dtc_bin);
                        }
                    } else {
                        g_print("Row contains widget type: %s\n", G_OBJECT_TYPE_NAME(child));
                    }
                }

                ptr = ptr_next;
            }
            for(int i = 0; i < dtcs->size; i++) {
                ad_buffer_append_melt(binResponse, dtcs->list[i]);
                dtcs->list[i] = null;
            }
            ad_list_Buffer_free(dtcs);
        }
    }
    return binResponse;
}
static void use_inverted_signal(SimECUGenerator *generator, Buffer * binResponse, char * signal_path) {
    SimECUGeneratorGui *gui = (SimECUGeneratorGui *)generator->context;
    ad_object_hashmap_string_Ptr * signals_widgets = generator->state;
    GtkWidget * signal_widget = null;
    for(int i = 0; i < signals_widgets->size; i++) {
        ad_object_string * key = signals_widgets->keys[i];
        if ( strcmp(key->data, signal_path) == 0 ) {
            signal_widget = GTK_WIDGET(((ad_object_Ptr*)signals_widgets->values[i])->value);
            break;
        }
    }
    if ( signal_widget == null ) {
        log_err("signal %s not linked", signal_path);
        return;
    }
    gdouble percent = counter_get_fraction(signal_widget);
    ad_object_vehicle_signal * signal = ad_signal_get(signal_path);
    double value = signal->rv_max - signal->rv_min;
    value = (percent * value) + signal->rv_min;
    Buffer * signal_inverted = ad_expr_reduce_invert(value, signal->rv_formula, null);
    ad_buffer_append_melt(binResponse, signal_inverted);
    char * txt = gprintf("%.2f %s", value, signal->unit);
    counter_set_label(signal_widget, txt);
    free(txt);
}
static Buffer * response_saej1979_pid(SimECUGenerator *generator, final byte pid, int frameNumber) {
    SimECUGeneratorGui *gui = (SimECUGeneratorGui *)generator->context;
    Buffer * binResponse = ad_buffer_new();
    bool defaultBehaviour = true;
    // Should append only bytes according to the PID, but for simplicity we just append random data
    switch(pid) {
        case 0xC0:
        case 0xA0:
        case 0x80:
        case 0x60:
        case 0x40:
        case 0x20:
        case 0x00: {
            ad_buffer_append_melt(binResponse, ad_buffer_from_ascii_hex("FFFFFFFF"));
            defaultBehaviour = false;
        } break;
        case 0x01: {
            gboolean is_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->dtcs.milOn));
            Buffer* status = ad_buffer_new();
            ad_buffer_pad(status, 4, 0x00);
            if ( ! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->dtcs.dtcCleared)) ) {
                GList *ptr = gtk_container_get_children(GTK_CONTAINER(gui->dtcs.listView));
                final int dtc_count = g_list_length(ptr);
                status->buffer[0] = dtc_count;
                g_list_free(ptr);
                status->buffer[0] |= is_checked << 7;
            }
            ad_buffer_append_melt(binResponse, status);
            defaultBehaviour = false;
        } break;
    }
    if ( defaultBehaviour ) {
        ad_object_vehicle_signal * signal = ad_signal_get_from_saej1979_pid(pid);
        if ( signal != null ) {
            use_inverted_signal(generator, binResponse, ad_object_vehicle_signal_get_exec_path(signal));
        }
    }
    return binResponse;
}
static Buffer * response_saej1979_vehicle_identification_request_info_type(SimECUGenerator * generator, byte infoType) {
    SimECUGeneratorGui *gui = (SimECUGeneratorGui *)generator->context;
    unsigned * seed = generator->context;
    switch(infoType) {
        case 0x00:                                          return ad_buffer_from_ascii_hex("FFFFFFFF");
        case 0x01:                                          return ad_buffer_from_ascii_hex("05");
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_VIN: {
            const gchar * vin = gtk_entry_get_text(gui->vin);
            if ( 0 < strlen(vin) ) {
                final Buffer * vinBuffer = ad_buffer_from_ascii((char*)vin);
                ad_buffer_pad(vinBuffer, 17, 0x00);
                return vinBuffer;
            }
        } break;
        case 0x03:                                          return ad_buffer_from_ascii_hex("01");
        case 0x04:                                          return ad_buffer_new_random_with_seed(16, seed);
        case 0x05:                                          return ad_buffer_from_ascii_hex("01");
        case 0x06:                                          return ad_buffer_new_random_with_seed(4, seed);
        case 0x07:                                          return ad_buffer_from_ascii_hex("01");
        case 0x08:                                          return ad_buffer_new_random_with_seed(4, seed);
        case 0x09:                                          return ad_buffer_from_ascii_hex("01");
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: {
            const gchar * ecuName = gtk_entry_get_text(gui->ecuName);
            if ( 0 < strlen(ecuName) ) {
                final Buffer * name = ad_buffer_from_ascii((char*)ecuName);
                ad_buffer_pad(name, 20, 0x00);
                return name;
            }
        } break;
    }
    return ad_buffer_new();
}
#define REGISTER_WIDGET_SIGNAL(generator, widget, signal_path) \
    { \
        if ( generator->state == null ) { \
            generator->state = ad_object_hashmap_string_Ptr_new(); \
        } \
        ad_object_hashmap_string_Ptr * signals_widgets = generator->state; \
        ad_object_string * signal = ad_object_string_new_from(signal_path); \
        ad_object_Ptr * widget_ptr = ad_object_Ptr_new_from(widget); \
        ad_object_hashmap_string_Ptr_set(signals_widgets, signal, widget_ptr); \
    }

static Buffer * response(SimECUGenerator *generator, final Buffer *binRequest) {
    SimECUGeneratorGui *gui = (SimECUGeneratorGui *)generator->context;
    final Buffer *binResponse = ad_buffer_new();
    if ( binRequest->size == 0 ) {
        return binResponse;
    }
    if ( ! sim_ecu_generator_fill_success(binResponse, binRequest) ) {
        return ad_buffer_new();
    }
    
    switch(binRequest->buffer[0]) {
        case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA:
        case OBD_SERVICE_SHOW_CURRENT_DATA:
            return generator->response_saej1979_pids(generator, binRequest);
        case OBD_SERVICE_SHOW_DTC:
            return generator->response_saej1979_dtcs_wrapper(generator, binRequest->buffer[0]);
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION:
            return generator->response_saej1979_vehicle_identification_request(generator, binRequest);
        case OBD_SERVICE_CLEAR_DTC: {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui->dtcs.dtcCleared), true);
            log_msg(LOG_DEBUG, "Clearing DTCs");
        } break;

    }
    return binResponse;
}
static char * context_to_string(SimECUGenerator * this) {
    return null;
}
static bool context_load_from_string(SimECUGenerator * this, char * context) {
    return false;
}
SimECUGenerator* sim_ecu_generator_new_gui() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->response = SIM_ECU_GENERATOR_RESPONSE(response);
    generator->type = strdup("gui");
    generator->flavour.is_Iso15765_4 = 0;
    generator->response_saej1979_pid = response_saej1979_pid;
    generator->response_saej1979_dtcs = response_saej1979_dtcs;
    generator->response_saej1979_vehicle_identification_request_info_type = response_saej1979_vehicle_identification_request_info_type;
    generator->context_load_from_string = SIM_ECU_GENERATOR_CONTEXT_LOAD_FROM_STRING(context_load_from_string);
    generator->context_to_string = SIM_ECU_GENERATOR_CONTEXT_TO_STRING(context_to_string);
    generator->state = null;
    return generator;
}

static void add_dtc(GtkButton *button, gpointer user_data) {
    SimECUGeneratorGui* simGui = (SimECUGeneratorGui*)user_data;
    const char *dtc_string = gtk_entry_get_text(simGui->dtcs.input);
    if ( saej1979_dtc_bin_from_string((char*)dtc_string) == null ) {
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(simGui->dtcs.invalidDtc),"%s: expected LXXXX where L is P,C,B,U",dtc_string);
        gtk_widget_show_on_main_thread(simGui->dtcs.invalidDtc);
    } else {
        GtkWidget *label = gtk_label_new(dtc_string);
        gtk_container_add((GtkContainer*)simGui->dtcs.listView,label);
        gtk_widget_show(label);
    }
}

gboolean sim_ecu_generator_gui_show_gsource(gpointer data) {
    SimECUGeneratorGui * gui = (SimECUGeneratorGui*)data;
    sim_ecu_generator_gui_show(gui);
    return false;
}

typedef struct SimSignalWidgetCtx {
    SimECUGeneratorGui *gui;
    char *signal_path;
    GtkWidget *label;
    GtkWidget *counter;
} SimSignalWidgetCtx;

static void signal_widget_add(SimECUGenerator *generator, char *signal_path, char *counter_image) {
    SimECUGeneratorGui *gui = (SimECUGeneratorGui *)generator->context;
    ad_object_vehicle_signal *signal;
    GtkWidget *item;
    GtkWidget *label;
    GtkWidget *widget;
    char *title;
    char *baseTitle;

    if (gui == NULL || signal_path == NULL) {
        return;
    }

    signal = ad_signal_get(signal_path);
    if (signal == NULL) {
        return;
    }

    if (signal->name != NULL && signal->unit != NULL && signal->unit[0] != '\0') {
        title = gprintf("%s (%s):", signal->name, signal->unit);
    } else if (signal->name != NULL) {
        title = gprintf("%s:", signal->name);
    } else {
        title = gprintf("%s:", signal_path);
    }

    item = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_halign(item, GTK_ALIGN_FILL);
    gtk_widget_set_valign(item, GTK_ALIGN_START);
    gtk_widget_set_hexpand(item, FALSE);
    gtk_widget_set_vexpand(item, FALSE);
    gtk_widget_set_margin_start(item, 4);
    gtk_widget_set_margin_end(item, 4);
    gtk_widget_set_margin_top(item, 4);
    gtk_widget_set_margin_bottom(item, 4);

    label = gtk_label_new(title);
    free(title);

    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(label, GTK_ALIGN_CENTER);

    widget = gtk_drawing_area_new();
    gtk_widget_set_hexpand(widget, FALSE);
    gtk_widget_set_vexpand(widget, FALSE);
    gtk_widget_set_size_request(widget, 100, 100);

    counter_init_modifiable(widget, counter_image, true);

    if (signal->unit != NULL && signal->unit[0] != '\0') {
        baseTitle = gprintf("%.2f %s", signal->rv_min, signal->unit);
    } else {
        baseTitle = gprintf("%.2f", signal->rv_min);
    }
    counter_set_label(widget, baseTitle);
    free(baseTitle);

    REGISTER_WIDGET_SIGNAL(generator, widget, signal_path);

    gtk_box_pack_start(GTK_BOX(item), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(item), widget, FALSE, FALSE, 0);

    gtk_flow_box_insert(gui->data.container, item, -1);

    gtk_widget_show(label);
    gtk_widget_show(widget);
    gtk_widget_show(item);
}

static void on_signal_add_clicked(GtkButton *button, gpointer userdata) {
    SimECUGenerator *generator = (SimECUGenerator *)userdata;
    SimECUGeneratorGui *gui = (SimECUGeneratorGui*)generator->context;
    const char *signal_path = gtk_combo_box_get_active_id(GTK_COMBO_BOX(gui->data.selector));
    if (signal_path == NULL || *signal_path == '\0') {
        return;
    }

    signal_widget_add(generator, (char *)signal_path, "counter_85_2_255_0_0_255.png");
}
SimECUGeneratorGui * sim_ecu_generator_gui_set_context(SimECUGenerator *generator, char * ecuDesignation) {

    gtk_init(0, NULL);
    ad_saej1979_data_register_signals();

    char *ui_dir = installation_folder_resolve("ui"), *elm327simUiPath;
    if (ui_dir == NULL) {
        log_msg(LOG_ERROR, "Data directory not found, try reinstalling the software");
        return NULL;
    }
    asprintf(&elm327simUiPath, "%s" PATH_FOLDER_DELIM "sim_ecu_generator_gui.glade", ui_dir);
    
    GtkBuilder *builder = gtk_builder_new_from_file(elm327simUiPath);
    free(ui_dir);
    free(elm327simUiPath);
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window-root"));
    char * title;
    asprintf(&title, "ECU %s generator", ecuDesignation);
    gtk_window_set_title(GTK_WINDOW(window), title);
    free(title);

    SimECUGeneratorGui *gui = (SimECUGeneratorGui *)malloc(sizeof(SimECUGeneratorGui));
    *gui = (SimECUGeneratorGui){
        .window = GTK_WIDGET(gtk_builder_get_object(builder, "window-root")),
        .dtcs = {
            .listView = GTK_LIST_BOX(gtk_builder_get_object(builder, "dtcs-list-view")),
            .input = GTK_ENTRY(gtk_builder_get_object(builder, "dtc-list-input")),
            .inputButton = GTK_BUTTON(gtk_builder_get_object(builder, "dtc-list-input-button")),
            .milOn = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "dtc-list-mil")),
            .dtcCleared = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "dtc-list-dtc-cleared")),
            .invalidDtc = GTK_WIDGET(gtk_builder_get_object(builder, "window-invalid-dtc"))
        },
        .data = {
            .add = GTK_BUTTON(gtk_builder_get_object(builder, "signal-add")),
            .container = GTK_FLOW_BOX(gtk_builder_get_object(builder, "signal-container")),
            .selector = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "signal-selector"))
        },
        .ecuName = GTK_ENTRY(gtk_builder_get_object(builder,"ecu-name")),
        .vin = GTK_ENTRY(gtk_builder_get_object(builder,"vin"))
    };
    generator->context = (void *)gui;
    gtk_flow_box_set_selection_mode(gui->data.container, GTK_SELECTION_NONE);
    assert(0 != g_signal_connect(G_OBJECT(gui->window), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), NULL));
    assert(0 != g_signal_connect(G_OBJECT(gui->dtcs.invalidDtc), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), NULL));
    assert(0 != g_signal_connect(gui->dtcs.inputButton, "clicked", G_CALLBACK(add_dtc), gui));
    assert(0 != g_signal_connect(gui->data.add, "clicked", G_CALLBACK(on_signal_add_clicked), generator));

    signal_widget_add(generator, "SAEJ1979.vehicle_speed", "counter_85_2_255_0_0_255.png");
    signal_widget_add(generator, "SAEJ1979.coolant_temp", "gaugehalf_225_5_255_0_0_255.png");
    signal_widget_add(generator, "SAEJ1979.engine_speed", "counter_85_2_255_0_0_255.png");

    ad_object_hashmap_string_vehicle_signal *signals = ad_signals_get();
    if (signals != NULL) {
        for (int i = 0; i < signals->size; i++) {
            const char *key;
            ad_object_vehicle_signal *signal;
            const char *label_text;

            key = signals->keys[i]->data;
            signal = signals->values[i];

            if (key == NULL || signal == NULL) {
                continue;
            }

            label_text = signal->name != NULL ? signal->name : key;
            gtk_combo_box_text_append(gui->data.selector, key, label_text);
        }
    }

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));

    return gui;
}
static gboolean present_window_false(gpointer w) {
    sleep(1);
    gtk_window_set_keep_above(GTK_WINDOW(w), false);
    return false;
}

SimECUGeneratorGui * sim_ecu_generator_gui_show(SimECUGeneratorGui *simGui) {
    gtk_widget_show(simGui->window);
    gtk_window_set_keep_above(GTK_WINDOW(simGui->window), true);
    g_idle_add(present_window_false, (gpointer)simGui->window);
    gtk_window_present(GTK_WINDOW(simGui->window));
    return simGui;
}