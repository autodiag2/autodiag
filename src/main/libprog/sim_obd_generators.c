#include "libprog/sim_obd_generators.h"

void ecu_saej1979_sim_generator_gui(ECUEmulationGenerator *generator, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin) {
    ELM327SimGui *gui = (ELM327SimGui *)generator->context;
    
    switch(obd_query_bin->buffer[0]) {
        case 0x01: {
            if ( 1 < obd_query_bin->size ) {            
                switch(obd_query_bin->buffer[1]) {
                    case 0x01: {
                        gboolean is_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->dtcs.milOn));
                        Buffer* status = buffer_new();
                        buffer_padding(status, 4, 0x00);
                        status->buffer[0] |= is_checked << 7;
                        buffer_append(responseOBDdataBin, status);
                    } break;
                    case 0x05: {
                        gdouble percent = counter_get_fraction(gui->data.coolantTemperature);
                        byte span = SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_MAX - SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_MIN;
                        int value = percent * span;
                        buffer_append_byte(responseOBDdataBin, (byte)(value));
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
                        buffer_append_byte(responseOBDdataBin, bA);
                        buffer_append_byte(responseOBDdataBin, bB);
                        char *res;
                        asprintf(&res, "%.2f r/min", value/4.0);
                        counter_set_label(gui->data.engineSpeed, res);
                        free(res);
                    } break;
                    case 0x0D: {
                        gdouble percent = counter_get_fraction(gui->data.vehicleSpeed);
                        byte span = SAEJ1979_DATA_VEHICULE_SPEED_MAX - SAEJ1979_DATA_VEHICULE_SPEED_MIN;
                        int value = percent * span;
                        buffer_append_byte(responseOBDdataBin, (byte)value);
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
                            char *dtc = gtk_label_get_text(GTK_LABEL(child));
                            Buffer *dtc_bin = saej1979_dtc_bin_from_string(dtc);
                            if ( dtc_bin == null ) {
                                log_msg(LOG_ERROR, "invalid dtc found");
                            } else {
                                buffer_append(responseOBDdataBin, dtc_bin);
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

ECUEmulationGenerator* sim_ecu_generator_new_gui() {
    ECUEmulationGenerator * generator = sim_ecu_generator_new();
    generator->obd_sim_response = SIM_ECU_GENERATOR_RESPONSE_FUNC(ecu_saej1979_sim_generator_gui);
    generator->type = strdup("gui");
    return generator;
}