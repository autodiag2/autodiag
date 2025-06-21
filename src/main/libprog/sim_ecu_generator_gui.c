#include "libprog/sim_ecu_generator_gui.h"

void sim_ecu_generator_response_gui(SimECUGenerator *generator, char ** response, final Buffer *binResponse, final Buffer *binRequest) {
    ELM327SimGui *gui = (ELM327SimGui *)generator->context;
    
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