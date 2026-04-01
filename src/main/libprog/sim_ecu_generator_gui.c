#include "libprog/sim_ecu_generator_gui.h"

static void coolant_temperature_set(SimECUGeneratorGui *gui, int temperature) {
    char *res;
    asprintf(&res, "%d °C", temperature);
    counter_set_label(gui->data.coolantTemperature, res);
    free(res);
}
static void engine_speed_set(SimECUGeneratorGui *gui, double speed) {
    char *res;
    asprintf(&res, "%.2f r/min", speed);
    counter_set_label(gui->data.engineSpeed, res);
    free(res);
}
static void vehicle_speed_set(SimECUGeneratorGui *gui, double speed) {
    char *res;
    asprintf(&res, "%.0f km/h", speed);
    counter_set_label(gui->data.vehicleSpeed, res);
    free(res);
}
static Buffer * saej1979_response_dtcs(SimECUGenerator *generator, int service_id) {
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
static Buffer * saej1979_response_pid(SimECUGenerator *generator, final byte pid, int frameNumber) {
    SimECUGeneratorGui *gui = (SimECUGeneratorGui *)generator->context;
    Buffer * binResponse = ad_buffer_new();
    // Should append only bytes according to the PID, but for simplicity we just append random data
    switch(pid) {
        case 0xC0:
        case 0xA0:
        case 0x80:
        case 0x60:
        case 0x40:
        case 0x20:
        case 0x00: {
            ad_buffer_append_melt(binResponse, ad_buffer_from_ascii_hex("FFFFFFFFFF"));
        } break;
        case 0x01: {
            gboolean is_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->dtcs.milOn));
            Buffer* status = ad_buffer_new();
            ad_buffer_padding(status, 4, 0x00);
            if ( ! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->dtcs.dtcCleared)) ) {
                GList *ptr = gtk_container_get_children(GTK_CONTAINER(gui->dtcs.listView));
                final int dtc_count = g_list_length(ptr);
                status->buffer[0] = dtc_count;
                g_list_free(ptr);
                status->buffer[0] |= is_checked << 7;
            }
            ad_buffer_append_melt(binResponse, status);
        } break;
        case 0x05: {
            gdouble percent = counter_get_fraction(gui->data.coolantTemperature);
            ad_object_vehicle_signal * signal = ad_signal_get("SAEJ1979.coolant_temp");
            byte span = signal->rv_max - signal->rv_min;
            int value = percent * span;
            ad_buffer_append_byte(binResponse, (byte)(value));
            coolant_temperature_set(gui, value + signal->rv_min);
        } break;
        case 0x0C: {
            gdouble percent = counter_get_fraction(gui->data.engineSpeed);
            ad_object_vehicle_signal * signal = ad_signal_get("SAEJ1979.engine_speed");
            double span = signal->rv_max - signal->rv_min;
            int value = percent * span * 4; // span * percent = (256 * A + B ) / 4
            byte bA = (0xFF00 & value) >> 8;
            byte bB = 0xFF & value;
            ad_buffer_append_byte(binResponse, bA);
            ad_buffer_append_byte(binResponse, bB);
            engine_speed_set(gui, value/4.0 + signal->rv_min);
        } break;
        case 0x0D: {
            gdouble percent = counter_get_fraction(gui->data.vehicleSpeed);
            ad_object_vehicle_signal * signal = ad_signal_get("SAEJ1979.vehicle_speed");
            byte span = signal->rv_max - signal->rv_min;
            int value = percent * span;
            ad_buffer_append_byte(binResponse, (byte)value);
            vehicle_speed_set(gui, value + signal->rv_min);
        } break;
    }
    return binResponse;
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
            return generator->saej1979_response_pids(generator, binRequest);
        case OBD_SERVICE_SHOW_DTC:
            return generator->saej1979_response_dtcs_wrapper(generator, binRequest->buffer[0]);
        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION: {
            if ( 1 < binRequest->size ) {
                switch(binRequest->buffer[1]) {
                    case 0x00: {
                        ad_buffer_append_melt(binResponse, ad_buffer_from_ascii_hex("FFFFFFFFFF"));
                    } break;
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_VIN: {
                        const gchar * vin = gtk_entry_get_text(gui->vin);
                        if ( 0 < strlen(vin) ) {
                            final Buffer * vinBuffer = ad_buffer_from_ascii((char*)vin);
                            ad_buffer_padding(vinBuffer, 17, 0x00);
                            ad_buffer_append_melt(binResponse, vinBuffer);
                        }
                    } break;
                    case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: {
                        const gchar * ecuName = gtk_entry_get_text(gui->ecuName);
                        if ( 0 < strlen(ecuName) ) {
                            final Buffer * name = ad_buffer_from_ascii((char*)ecuName);
                            ad_buffer_padding(name, 20, 0x00);
                            ad_buffer_append_melt(binResponse, name);
                        }
                    } break;
                }
            }
        } break;
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
    generator->saej1979_response_pid = saej1979_response_pid;
    generator->saej1979_response_dtcs = saej1979_response_dtcs;
    generator->context_load_from_string = SIM_ECU_GENERATOR_CONTEXT_LOAD_FROM_STRING(context_load_from_string);
    generator->context_to_string = SIM_ECU_GENERATOR_CONTEXT_TO_STRING(context_to_string);
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
        },
        .ecuName = GTK_ENTRY(gtk_builder_get_object(builder,"ecu-name")),
        .vin = GTK_ENTRY(gtk_builder_get_object(builder,"vin"))
    };

    assert(0 != g_signal_connect(G_OBJECT(simGui->window), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), NULL));
    assert(0 != g_signal_connect(G_OBJECT(simGui->dtcs.invalidDtc), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), NULL));
    assert(0 != g_signal_connect(simGui->dtcs.inputButton, "clicked", G_CALLBACK(add_dtc), simGui));

    counter_init_modifiable(GTK_WIDGET(simGui->data.vehicleSpeed),"counter_85_2_255_0_0_255.png", true);
    counter_init_modifiable(GTK_WIDGET(simGui->data.coolantTemperature),"gaugehalf_225_5_255_0_0_255.png", true);
    counter_init_modifiable(GTK_WIDGET(simGui->data.engineSpeed),"counter_85_2_255_0_0_255.png", true);

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));

    coolant_temperature_set(simGui, NAN);
    engine_speed_set(simGui, NAN);
    vehicle_speed_set(simGui, NAN);

    generator->context = (void *)simGui;

    return simGui;
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