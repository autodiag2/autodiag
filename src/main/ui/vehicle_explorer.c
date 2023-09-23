#include "ui/vehicle_explorer.h"

vehicleExplorerGui *vdgui = null;
pthread_t *vehicle_explorer_updater_thread = null;

gboolean vehicle_explorer_onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused) {
   module_debug(MODULE_VEHICULE_DIAGNOSTIC "Close event received");
   if ( vehicle_explorer_updater_thread != null ) {
        pthread_cancel(*vehicle_explorer_updater_thread);
        free(vehicle_explorer_updater_thread);
        vehicle_explorer_updater_thread = null;
   }
   gtk_widget_hide(vdgui->window);
   return true;
}

#define INFO_NO_OBD_PORT_SELECTED() log_msg("refresh aborted : not obd iface selected", LOG_INFO)

void vehicle_explorer_fill_label(GtkLabel *label, const char *format, double value) {
    char *value_txt;
    if ( asprintf(&value_txt, format, value) == -1 ) {
        log_msg("Fill label impossible",LOG_ERROR);
    } else {
        gtk_label_set_text(label, value_txt);
        free(value_txt);
    }
}

#define _vehicle_explorer_fill_progressbar_widget(bar,value,min,max,error,format)\
    char * value_txt;\
    if ( asprintf(&value_txt, format, value) == -1 ) {\
        log_msg("Fill progress bar impossible",LOG_ERROR);\
    } else {\
        if ( strcmp(value_txt,gtk_progress_bar_get_text(bar)) == 0 ) {\
                \
        } else {\
          if ( value == error ) {\
          \
          } else { \
            final double value_percent = compute_normalized_of(value,min,max);\
            if ( log_get_level() == LOG_DEBUG ) {\
                if ( value_percent < 0 || 1 < value_percent ) { \
                    char *msg;\
                    asprintf(&msg, "out of range value detected value_percent=%.2f value=%d min=%d max=%d\n", value_percent, (int)value, (int)min, (int)max);\
                    log_msg(msg, LOG_DEBUG);\
                    free(msg);\
                }\
            }\
            gtk_progress_bar_set_text(bar, value_txt);\
            gtk_progress_bar_set_fraction(bar, value_percent);\
          }\
        }\
        free(value_txt);\
    }
void vehicle_explorer_fill_progressbar_widget_from_int(GtkProgressBar *bar, int value, int min, int max, int error, char *format) {
    _vehicle_explorer_fill_progressbar_widget(bar,value,min,max,error,format);
}

void vehicle_explorer_fill_progressbar_widget_from_double(GtkProgressBar *bar, double value, double min, double max, double error, char *format) {
    _vehicle_explorer_fill_progressbar_widget(bar,value,min,max,error,format);
}

void vehicle_explorer_fill_progressbar_widget(GtkProgressBar *bar, int value, int min, int max, int error, char *format) {
    vehicle_explorer_fill_progressbar_widget_from_int(bar,value,min,max,error,format);
}

void vehicle_explorer_fill_engine_fuel_trim_widget(GtkProgressBar *bar, int fuel_trim) {
    vehicle_explorer_fill_progressbar_widget(bar,fuel_trim,SAEJ1979_FUEL_TRIM_MIN,SAEJ1979_FUEL_TRIM_MAX,SAEJ1979_FUEL_TRIM_ERROR,"%d %%");
}

void vehicle_explorer_dynamic_refresh_information() {
    final OBDIFACE iface = config.ephemere.iface;
    if ( iface == null ) {
        INFO_NO_OBD_PORT_SELECTED();
    } else {
        if ( gtk_widget_get_mapped(GTK_WIDGET(vdgui->engine.fuel.pressure)) ) {
            vehicle_explorer_fill_progressbar_widget(vdgui->engine.fuel.pressure,
                saej1979_get_current_fuel_pressure(iface),
                SAEJ1979_FUEL_PRESSURE_MIN,SAEJ1979_FUEL_PRESSURE_MAX,SAEJ1979_FUEL_PRESSURE_ERROR,
                "%d kPa"
            );
        }
        if ( gtk_widget_get_mapped(GTK_WIDGET(vdgui->engine.fuel.level)) ) {
            vehicle_explorer_fill_progressbar_widget_from_double(vdgui->engine.fuel.level,
                saej1979_get_current_fuel_tank_level_input(iface),
                SAEJ1979_FUEL_TANK_LEVEL_INPUT_MIN,SAEJ1979_FUEL_TANK_LEVEL_INPUT_MAX,
                SAEJ1979_FUEL_TANK_LEVEL_INPUT_ERROR,
                "%.2f %%"
            );
        }
        if ( gtk_widget_get_mapped(GTK_WIDGET(vdgui->engine.coolant.temperature)) ) {
            vehicle_explorer_fill_progressbar_widget(vdgui->engine.coolant.temperature,
                saej1979_get_current_engine_coolant_temperature(iface),
                SAEJ1979_ENGINE_COOLANT_TEMPERATURE_MIN,SAEJ1979_ENGINE_COOLANT_TEMPERATURE_MAX,SAEJ1979_ENGINE_COOLANT_TEMPERATURE_ERROR,
                "%d °C"
            );
        }
        if ( gtk_widget_get_mapped(GTK_WIDGET(vdgui->engine.intakeAir.temperature)) ) {
            vehicle_explorer_fill_progressbar_widget(vdgui->engine.intakeAir.temperature,
                saej1979_get_current_intake_air_temperature(iface),
                SAEJ1979_INTAKE_AIR_TEMPERATURE_MIN,SAEJ1979_INTAKE_AIR_TEMPERATURE_MAX,SAEJ1979_INTAKE_AIR_TEMPERATURE_ERROR,
                "%d °C"
            );
        }
        if ( gtk_widget_get_mapped(GTK_WIDGET(vdgui->engine.speed)) ) {
            vehicle_explorer_fill_progressbar_widget_from_double(vdgui->engine.speed,
                saej1979_get_current_engine_speed(iface),
                SAEJ1979_ENGINE_SPEED_MIN,SAEJ1979_ENGINE_SPEED_MAX,SAEJ1979_ENGINE_SPEED_ERROR,
                "%.2f r/min"
            );
        }
        if ( gtk_widget_get_mapped(GTK_WIDGET(vdgui->engine.ecu.voltage)) ) {
            final double currentECUvoltage = saej1979_get_current_ecu_voltage(iface);
            if ( currentECUvoltage != SAEJ1979_ECU_VOLTAGE_ERROR ) {
                vehicle_explorer_fill_label(vdgui->engine.ecu.voltage,
                    "%.3f V", currentECUvoltage
                );
            }
        }
        if ( gtk_widget_get_mapped(GTK_WIDGET(vdgui->engine.fuel.trim.longTerm.bank1)) ) {
            vehicle_explorer_fill_engine_fuel_trim_widget(vdgui->engine.fuel.trim.longTerm.bank1, saej1979_get_current_long_term_fuel_trim_bank_1(iface));
        }
        if ( gtk_widget_get_mapped(GTK_WIDGET(vdgui->engine.fuel.trim.shortTerm.bank1)) ) {
            vehicle_explorer_fill_engine_fuel_trim_widget(vdgui->engine.fuel.trim.shortTerm.bank1, saej1979_get_current_short_term_fuel_trim_bank_1(iface));
        }
        if ( gtk_widget_get_mapped(GTK_WIDGET(vdgui->engine.fuel.trim.longTerm.bank2)) ) {
            vehicle_explorer_fill_engine_fuel_trim_widget(vdgui->engine.fuel.trim.longTerm.bank2, saej1979_get_current_long_term_fuel_trim_bank_2(iface));
        }
        if ( gtk_widget_get_mapped(GTK_WIDGET(vdgui->engine.fuel.trim.shortTerm.bank2)) ) {
            vehicle_explorer_fill_engine_fuel_trim_widget(vdgui->engine.fuel.trim.shortTerm.bank2, saej1979_get_current_short_term_fuel_trim_bank_2(iface));
        }
    }
}

void vehicle_explorer_one_time_refresh_information() {
    final OBDIFACE iface = config.ephemere.iface;
    if ( iface == null ) {
        INFO_NO_OBD_PORT_SELECTED();
    } else {
        {
            final char * txt = saej1979_get_current_engine_type_as_string(iface);
            gtk_label_set_text(vdgui->engine.type, txt);
            free(txt);
        }    
        {
            final char *txt = saej1979_get_current_obd_standard_str(iface);
            gtk_label_set_text(vdgui->engine.ecu.obdStandard, txt);
            free(txt);
        }
    }
}

void* vehicle_explorer_daemon_updater(void *arg) {
    pthread_cleanup_push(obd_thread_cleanup_routine, null);
    const struct timespec req = {
        .tv_sec = config.vehicleExplorer.refreshRateS,
        .tv_nsec = (config.vehicleExplorer.refreshRateS - ((int)config.vehicleExplorer.refreshRateS)) * 1000000000
    };
    while(true) {
        vehicle_explorer_dynamic_refresh_information();
        if ( nanosleep(&req,null) == -1 ) {
            break;
        }
    }
    pthread_cleanup_pop(1);
}

void vehicle_explorer_global_refresh() {
    vehicle_explorer_dynamic_refresh_information(); 
    vehicle_explorer_one_time_refresh_information();  
}

void* vehicle_explorer_daemon_one_time_refresh(void *arg) {
    vehicle_explorer_global_refresh();
}

void vehicle_explorer_start_daemon_one_time_refresh() {
    pthread_t t;
    pthread_create(&t, null, &vehicle_explorer_daemon_one_time_refresh, null);
}

void* vehicle_explorer_daemon_one_time_refresh_spinner(final void *arg) {
    final pthread_t * t = (pthread_t*)arg;
    gtk_spinner_start(vdgui->refreshIcon);
    pthread_join(*t, NULL);
    gtk_spinner_stop(vdgui->refreshIcon);
    free(t);
}
void vehicle_explorer_start_daemon_one_time_refresh_with_spinner() {
    pthread_t *t,t1;
    t = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(t, null, &vehicle_explorer_daemon_one_time_refresh, null);
    pthread_create(&t1, null, &vehicle_explorer_daemon_one_time_refresh_spinner, t);
}

void show_window_vehicle_explorer() {
    gtk_widget_show_now (vdgui->window);
    vehicle_explorer_start_daemon_one_time_refresh();
    if ( vehicle_explorer_updater_thread == null ) {
        vehicle_explorer_updater_thread = (pthread_t*)malloc(sizeof(pthread_t));
        if ( pthread_create(vehicle_explorer_updater_thread, null, &vehicle_explorer_daemon_updater, null) == 0 ) {
            module_debug(MODULE_VEHICULE_DIAGNOSTIC "Checker thread created");
        } else {
            module_debug(MODULE_VEHICULE_DIAGNOSTIC "Fatal erro during thead creation");
        }
    }
}

gboolean vehicle_explorer_set_widget_visible_true_callback(gpointer data) {
    GtkWidget *widget = (GtkWidget *)data;
    gtk_widget_set_mapped(widget, true);
    gtk_widget_set_visible(widget, true);
    gtk_widget_realize(widget);
    return false;
}
void vehicle_explorer_mapped_true_one_level_callback(GtkWidget *widget, gpointer data) {
    final const char * name = gtk_widget_get_name(widget);
    if ( strcmp(name,"GtkBox") == 0 ) {
        gtk_container_foreach((GtkContainer*)widget,
            vehicle_explorer_mapped_true_one_level_callback,
            null);
    } else if ( strcmp(name,"GtkProgressBar") == 0 ) {
        g_idle_add(vehicle_explorer_set_widget_visible_true_callback, widget);
    }
}
void vehicle_explorer_mapped_true_one_level(GtkExpander* expander, gpointer data) {
    final bool state = gtk_expander_get_expanded(expander);
    if ( ! state ) {
        gtk_container_foreach((GtkContainer*)expander,
            vehicle_explorer_mapped_true_one_level_callback,
            null);
    }
}

void vehicle_explorer_mapped_false(GtkWidget *widget, gpointer data) {
    final bool mapped = false;
    final const char * name = gtk_widget_get_name(widget);
    if ( strcmp(name,"GtkBox") == 0 || strcmp(name,"GtkExpander") == 0 ) {
        if ( strcmp(name,"GtkExpander") == 0 ) {
            g_signal_connect(G_OBJECT(widget),"activate",G_CALLBACK(vehicle_explorer_mapped_true_one_level),NULL);
        }
        gtk_container_foreach((GtkContainer*)widget,
            vehicle_explorer_mapped_false,
            &mapped);
    } else if ( strcmp(name,"GtkProgressBar") == 0 ) {
        gtk_widget_set_mapped(widget,mapped);
        gtk_widget_set_visible(widget,mapped);
    } else {
        
    }
}

void module_init_vehicle_explorer(final GtkBuilder *builder) {
    if ( vdgui == null ) {
        vehicleExplorerGui g = {
            .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-vehicle-explorer")),
            .refreshIcon = (GtkSpinner*)gtk_builder_get_object (builder, "window-vehicle-explorer-global-refresh"),
            .engine = {
                .expandableSection = (GtkBox *)gtk_builder_get_object(builder,"vehicle-explorer-expandable-section"),
                .speed = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-speed-graph"),
                .type = (GtkLabel*)gtk_builder_get_object (builder, "vehicle-explorer-engine-type"),
                .intakeAir = {
                    .temperature = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-intake-air-temperature")
                },
                .fuel = {
                    .pressure = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-pressure"),
                    .level = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-level"),
                    .trim = {
                        .longTerm = {
                            .bank1 = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-trim-long-term-1"),
                            .bank2 = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-trim-long-term-2")
                        },
                        .shortTerm = {
                            .bank1 = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-trim-short-term-1"),
                            .bank2 = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-trim-short-term-2")
                        }
                    }
                },
                .coolant = {
                    .temperature = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-coolant-temperature-graph")
                },
                .ecu = {
                    .voltage = (GtkLabel*)gtk_builder_get_object (builder, "vehicle-explorer-ecu-voltage"),
                    .obdStandard = (GtkLabel*)gtk_builder_get_object (builder, "vehicle-explorer-ecu-obd-standard")
                }
            }
        };
        vdgui = (vehicleExplorerGui*)malloc(sizeof(vehicleExplorerGui));
        (*vdgui) = g;
        vehicle_explorer_mapped_false((GtkWidget*)vdgui->engine.expandableSection,null);
        g_signal_connect(G_OBJECT(vdgui->window),"delete-event",G_CALLBACK(vehicle_explorer_onclose),NULL);
        gtk_builder_add_callback_symbol(builder,"show-window-vehicle-explorer",&show_window_vehicle_explorer);
        gtk_builder_add_callback_symbol(builder,"window-vehicle-explorer-global-refresh-click",&vehicle_explorer_start_daemon_one_time_refresh_with_spinner);
    } else {
        module_debug(MODULE_VEHICULE_DIAGNOSTIC "module already initialized");
    }
}


