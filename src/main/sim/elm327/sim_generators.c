#include "sim/elm327/sim_generators.h"
#include "sim/elm327/elm327_cli.h"

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

int ecu_saej1979_sim_generator_cycle_percent[0xFF][0xFF] = {0};
void ecu_saej1979_sim_generator_cycle_iterate(int service_id, int pid, unsigned gears) {
    ecu_saej1979_sim_generator_cycle_percent[service_id][pid] += (100/gears);
    ecu_saej1979_sim_generator_cycle_percent[service_id][pid] %= 100;
}
void ecu_saej1979_sim_generator_cycle(ECUEmulationGenerator *generator, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin) {
    unsigned gears = 10;
    if ( generator->context != null ) {
        gears = *((unsigned*)generator->context);
    }
    switch(obd_query_bin->buffer[0]) {
        case 0x02: case 0x01: {
            buffer_append(responseOBDdataBin,buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));
        } break;
        case 0x07: case 0x0A: case 0x03: {
            buffer_append(responseOBDdataBin,buffer_new_cycle(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][0]));                
        } break;
        case 0x04: {
            (*response) = strdup(SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
        } break;
        case 0x09: {
            if ( 1 < obd_query_bin->size ) {            
                switch(obd_query_bin->buffer[1]) {
                    case 0x00: {
                        buffer_append(responseOBDdataBin, buffer_from_ascii_hex("FFFFFFFF"));
                        break;
                    }
                    case 0x01: {
                        buffer_append_byte(responseOBDdataBin, 0x05);
                        break;
                    }
                    case 0x02: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(17, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));                
                        break;
                    }
                    case 0x03: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x04: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(16, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));                
                        break;
                    }
                    case 0x05: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x06: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(4, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));
                        break;
                    }
                    case 0x07: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x08: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(4, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));
                        break;
                    }
                    case 0x09: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x0A: {
                        final Buffer * name = buffer_from_ascii("TEST");
                        buffer_padding(name, 20, 0x00);
                        buffer_append(responseOBDdataBin, name);
                        break;
                    }
                    case 0x0B: {
                        buffer_append(responseOBDdataBin,buffer_new_cycle(4, ecu_saej1979_sim_generator_cycle_percent[obd_query_bin->buffer[0]][obd_query_bin->buffer[1]]));
                        break;
                    }
                }
            } else {
                log_msg(LOG_ERROR, "Missing PID");
            }
        } break;
    }
    ecu_saej1979_sim_generator_cycle_iterate(obd_query_bin->buffer[0], 1 < obd_query_bin->size ? obd_query_bin->buffer[1] : 0, gears);
}

void ecu_saej1979_sim_generator_random(ECUEmulationGenerator *generator, char ** response, final Buffer *responseOBDdataBin, final Buffer *obd_query_bin) {
    unsigned * seed = generator->context;
    if ( seed == null ) {
        seed = (unsigned*)malloc(sizeof(unsigned));
        *seed = 1;
        generator->context = seed;
    }
    switch(obd_query_bin->buffer[0]) {
        case 0x02: case 0x01: {
            buffer_append(responseOBDdataBin,buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 2, seed));
        } break;
        case 0x07: case 0x0A: case 0x03: {
            buffer_append(responseOBDdataBin,buffer_new_random_with_seed(ISO_15765_SINGLE_FRAME_DATA_BYTES - 1, seed));                
        } break;
        case 0x04: {
            (*response) = strdup(SerialResponseStr[SERIAL_RESPONSE_OK-SerialResponseOffset]);
        } break;
        case 0x09: {
            if ( 1 < obd_query_bin->size ) {            
                switch(obd_query_bin->buffer[1]) {
                    case 0x00: {
                        buffer_append(responseOBDdataBin, buffer_from_ascii_hex("FFFFFFFF"));
                        break;
                    }
                    case 0x01: {
                        buffer_append_byte(responseOBDdataBin, 0x05);
                        break;
                    }
                    case 0x02: {
                        buffer_append(responseOBDdataBin,buffer_new_random_with_seed(17, seed));                
                        break;
                    }
                    case 0x03: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x04: {
                        buffer_append(responseOBDdataBin,buffer_new_random_with_seed(16, seed));                
                        break;
                    }
                    case 0x05: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x06: {
                        buffer_append(responseOBDdataBin,buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x07: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x08: {
                        buffer_append(responseOBDdataBin,buffer_new_random_with_seed(4, seed));
                        break;
                    }
                    case 0x09: {
                        buffer_append_byte(responseOBDdataBin,0x01);
                        break;
                    }
                    case 0x0A: {
                        final Buffer * name = buffer_from_ascii("TEST");
                        buffer_padding(name, 20, 0x00);
                        buffer_append(responseOBDdataBin, name);
                        break;
                    }
                    case 0x0B: {
                        buffer_append(responseOBDdataBin,buffer_new_random_with_seed(4, seed));
                        break;
                    }
                }
            } else {
                log_msg(LOG_ERROR, "Missing PID");
            }
        } break;
    }
}