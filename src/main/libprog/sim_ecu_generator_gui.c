#include "libprog/sim_ecu_generator_gui.h"

void sim_ecu_generator_response_gui(SimECUGenerator *generator, char ** response, final Buffer *binResponse, final Buffer *binRequest) {
    SimECUGeneratorGui *gui = (SimECUGeneratorGui *)generator->context;
    
    switch(binRequest->buffer[0]) {
        case 0x01: {
            if ( 1 < binRequest->size ) {            
                switch(binRequest->buffer[1]) {
                    case 0x01: {
                        gboolean is_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->dtcs.milOn));
                        Buffer* status = buffer_new();
                        buffer_padding(status, 4, 0x00);
                        if ( ! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->dtcs.dtcCleared)) ) {
                            GList *ptr = gtk_container_get_children(GTK_CONTAINER(gui->dtcs.listView));
                            final int dtc_count = g_list_length(ptr);
                            status->buffer[0] = dtc_count;
                            g_list_free(ptr);
                            status->buffer[0] |= is_checked << 7;
                        }
                        buffer_append(binResponse, status);
                    } break;
                    case 0x05: {
                        gdouble percent = counter_get_fraction(gui->data.coolantTemperature);
                        byte span = SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_MAX - SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_MIN;
                        int value = percent * span;
                        buffer_append_byte(binResponse, (byte)(value));
                        char *res;
                        asprintf(&res, "%d °C", value - 40);
                        counter_set_label(gui->data.coolantTemperature, res);
                        free(res);
                    } break;
                    case 0x0C: {
                        gdouble percent = counter_get_fraction(gui->data.engineSpeed);
                        double span = SAEJ1979_DATA_ENGINE_SPEED_MAX - SAEJ1979_DATA_ENGINE_SPEED_MIN;
                        int value = percent * span * 4; // span * percent = (256 * A + B ) / 4
                        byte bA = (0xFF00 & value) >> 8;
                        byte bB = 0xFF & value;
                        buffer_append_byte(binResponse, bA);
                        buffer_append_byte(binResponse, bB);
                        char *res;
                        asprintf(&res, "%.2f r/min", value/4.0);
                        counter_set_label(gui->data.engineSpeed, res);
                        free(res);
                    } break;
                    case 0x0D: {
                        gdouble percent = counter_get_fraction(gui->data.vehicleSpeed);
                        byte span = SAEJ1979_DATA_VEHICULE_SPEED_MAX - SAEJ1979_DATA_VEHICULE_SPEED_MIN;
                        int value = percent * span;
                        buffer_append_byte(binResponse, (byte)value);
                        char *res;
                        asprintf(&res, "%d km/h", value);
                        counter_set_label(gui->data.vehicleSpeed, res);
                        free(res);
                    } break;
                }
            }
        } break;
        case 0x03: {
            if ( ! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->dtcs.dtcCleared)) ) {
                GList *ptr = gtk_container_get_children(GTK_CONTAINER(gui->dtcs.listView));
                while (ptr != NULL) {
                    GList *ptr_next = ptr->next;
                    GtkWidget *row = GTK_WIDGET(ptr->data);

                    if (GTK_IS_LIST_BOX_ROW(row)) {
                        GtkWidget *child = gtk_bin_get_child(GTK_BIN(row));
                        if (GTK_IS_LABEL(child)) {
                            const char *dtc = gtk_label_get_text(GTK_LABEL(child));
                            Buffer *dtc_bin = saej1979_dtc_bin_from_string(dtc);
                            if ( dtc_bin == null ) {
                                log_msg(LOG_ERROR, "invalid dtc found");
                            } else {
                                buffer_append(binResponse, dtc_bin);
                            }
                        } else {
                            g_print("Row contains widget type: %s\n", G_OBJECT_TYPE_NAME(child));
                        }
                    }

                    ptr = ptr_next;
                }
            }
        } break;
        case 0x04: {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui->dtcs.dtcCleared), true);
            (*response) = strdup(SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
        } break;

    }

}

SimECUGenerator* sim_ecu_generator_new_gui() {
    SimECUGenerator * generator = sim_ecu_generator_new();
    generator->sim_ecu_generator_response = SIM_ECU_GENERATOR_RESPONSE_FUNC(sim_ecu_generator_response_gui);
    generator->type = strdup("gui");
    return generator;
}

void sim_ecu_generator_gui_add_dtc(GtkButton *button, gpointer user_data) {
    SimECUGeneratorGui* simGui = (SimECUGeneratorGui*)user_data;
    const char *dtc_string = gtk_entry_get_text(simGui->dtcs.input);
    if ( saej1979_dtc_bin_from_string(dtc_string) == null ) {
        gtk_message_dialog_format_secondary_text(simGui->dtcs.invalidDtc,"%s: expected LXXXX where L is P,C,B,U",dtc_string);
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

SimECUGeneratorGui * sim_ecu_generator_gui_set_context(SimECUGenerator *generator, char * ecuDesignation) {

    gtk_init(0, NULL);

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

    SimECUGeneratorGui *simGui = (SimECUGeneratorGui *)malloc(sizeof(SimECUGeneratorGui));
    *simGui = (SimECUGeneratorGui){
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
            .vehicleSpeed = GTK_WIDGET(gtk_builder_get_object(builder, "data-vehicle-speed")),
            .coolantTemperature = GTK_WIDGET(gtk_builder_get_object(builder, "data-coolant-temperature")),
            .engineSpeed = GTK_WIDGET(gtk_builder_get_object(builder, "data-engine-speed"))
        }
    };

    g_signal_connect(G_OBJECT(simGui->window), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), NULL);
    g_signal_connect(G_OBJECT(simGui->dtcs.invalidDtc), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), NULL);
    g_signal_connect(simGui->dtcs.inputButton, "clicked", G_CALLBACK(sim_ecu_generator_gui_add_dtc), simGui);

    counter_init_modifiable(simGui->data.vehicleSpeed,"counter_85_2_255_0_0_255.png", true);
    counter_init_modifiable(simGui->data.coolantTemperature,"gaugehalf_225_5_255_0_0_255.png", true);
    counter_init_modifiable(simGui->data.engineSpeed,"counter_85_2_255_0_0_255.png", true);

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));

    generator->context = (void *)simGui;

    return simGui;
}
gboolean sim_ecu_generator_gui_present_window_false(gpointer w) {
    sleep(1);
    gtk_window_set_keep_above(GTK_WINDOW(w), false);
    return false;
}

SimECUGeneratorGui * sim_ecu_generator_gui_show(SimECUGeneratorGui *simGui) {
    gtk_widget_show(simGui->window);
    gtk_window_set_keep_above(GTK_WINDOW(simGui->window), true);
    g_idle_add(sim_ecu_generator_gui_present_window_false, (gpointer)simGui->window);
    gtk_window_present(GTK_WINDOW(simGui->window));
}