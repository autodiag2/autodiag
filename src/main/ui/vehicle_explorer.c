#include "ui/vehicle_explorer.h"

vehicleExplorerGui *vdgui = null;
pthread_t *vehicle_explorer_refresh_dynamic_thread = null;

static void vehicle_explorer_button_click_clean_up_routine(void *arg) {
    obd_thread_cleanup_routine(arg);
    gtk_spinner_stop(vdgui->refreshIcon);
}

bool vehicle_explorer_show_freeze_frame_get_state() {
    return gtk_check_menu_item_get_active(vdgui->menuBar.showFreezeFrame);    
}

void vehicle_explorer_freeze_frame_error_ok() {
    gtk_widget_hide_on_main_thread(GTK_WIDGET(vdgui->menuBar.freeze_frame_error_popup));
}

void vehicle_explorer_data_freeze_frame_internal() {
    final OBDIFace* iface = config.ephemere.iface;
    if ( vehicle_explorer_error_feedback_obd(iface) ) {
        return;
    } else {
        gtk_spinner_start(vdgui->refreshIcon);
        if ( ! saej1979_data_freeze_frame(iface) ) {
            gtk_widget_show_on_main_thread(GTK_WIDGET(vdgui->menuBar.freeze_frame_error_popup));
        }
    }
}
THREAD_WRITE_DAEMON(
    vehicle_explorer_data_freeze_frame_daemon,
    vehicle_explorer_data_freeze_frame_internal,
    vehicle_explorer_button_click_clean_up_routine, vdgui->menuBar.freeze_frame_thread
)

void vehicle_explorer_data_freeze_frame() {
    thread_allocate_and_start(&vdgui->menuBar.freeze_frame_thread,&vehicle_explorer_data_freeze_frame_daemon);    
}

gboolean vehicle_explorer_onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused) {
   module_debug(MODULE_VEHICULE_DIAGNOSTIC "Close event received");
   THREAD_CANCEL(vehicle_explorer_refresh_dynamic_thread);
   gtk_widget_hide(vdgui->window);
   return true;
}

#define VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(sym,type,min,max,error,format,bar) \
    GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(vehicle_explorer_##sym##_gsource,type,min,max,error,format,bar)

#define VH_SHOULD_REFRESH_WIDGET(widget) \
    ( \
        ( \
            strcmp(gtk_widget_get_name(GTK_WIDGET(widget)),"GtkExpander") == 0 ? \
                gtk_expander_get_expanded(GTK_EXPANDER(widget)) : \
                true \
        ) && gtk_widget_get_mapped(GTK_WIDGET(widget)) \
    )
    

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_engine_coolant_temperature,
    int, SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_MIN,
    SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_MAX,SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_ERROR,
    "%d °C", vdgui->engine.coolant.temperature
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_engine_speed,
    double,
    SAEJ1979_DATA_ENGINE_SPEED_MIN,SAEJ1979_DATA_ENGINE_SPEED_MAX,SAEJ1979_DATA_ENGINE_SPEED_ERROR,
    "%.2f r/min", vdgui->engine.speed
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_fuel_pressure,
    int,
    SAEJ1979_DATA_FUEL_PRESSURE_MIN,SAEJ1979_DATA_FUEL_PRESSURE_MAX,SAEJ1979_DATA_FUEL_PRESSURE_ERROR,
    "%d kPa", vdgui->engine.fuel.pressure
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_fuel_tank_level_input,
    double,
    SAEJ1979_DATA_FUEL_TANK_LEVEL_INPUT_MIN,SAEJ1979_DATA_FUEL_TANK_LEVEL_INPUT_MAX,
    SAEJ1979_DATA_FUEL_TANK_LEVEL_INPUT_ERROR, "%.2f %%", vdgui->engine.fuel.level
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_intake_air_temperature,
    int,
    SAEJ1979_DATA_INTAKE_AIR_TEMPERATURE_MIN,SAEJ1979_DATA_INTAKE_AIR_TEMPERATURE_MAX,SAEJ1979_DATA_INTAKE_AIR_TEMPERATURE_ERROR,
    "%d °C", vdgui->engine.intakeAir.temperature
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_intake_manifold_pressure,
    int,
    SAEJ1979_DATA_INTAKE_MANIFOLD_PRESSURE_MIN,SAEJ1979_DATA_INTAKE_MANIFOLD_PRESSURE_MAX,SAEJ1979_DATA_INTAKE_MANIFOLD_PRESSURE_ERROR,
    "%d kPa", vdgui->engine.intakeAir.manifoldPressure
)
VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_maf_air_flow_rate,
    double,
    SAEJ1979_DATA_VEHICULE_MAF_AIR_FLOW_RATE_MIN,SAEJ1979_DATA_VEHICULE_MAF_AIR_FLOW_RATE_MAX,SAEJ1979_DATA_VEHICULE_MAF_AIR_FLOW_RATE_ERROR,
    "%.2f g/s", vdgui->engine.intakeAir.mafRate
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_long_term_fuel_trim_bank_1,
    int,
    SAEJ1979_DATA_FUEL_TRIM_MIN,SAEJ1979_DATA_FUEL_TRIM_MAX,SAEJ1979_DATA_FUEL_TRIM_ERROR,"%d %%", 
    vdgui->engine.fuel.trim.longTerm.bank1
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_long_term_fuel_trim_bank_2,
    int,
    SAEJ1979_DATA_FUEL_TRIM_MIN,SAEJ1979_DATA_FUEL_TRIM_MAX,SAEJ1979_DATA_FUEL_TRIM_ERROR,"%d %%", 
    vdgui->engine.fuel.trim.longTerm.bank2
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_short_term_fuel_trim_bank_1,
    int,
    SAEJ1979_DATA_FUEL_TRIM_MIN,SAEJ1979_DATA_FUEL_TRIM_MAX,SAEJ1979_DATA_FUEL_TRIM_ERROR,"%d %%", 
    vdgui->engine.fuel.trim.shortTerm.bank1
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_short_term_fuel_trim_bank_2,
    int,
    SAEJ1979_DATA_FUEL_TRIM_MIN,SAEJ1979_DATA_FUEL_TRIM_MAX,SAEJ1979_DATA_FUEL_TRIM_ERROR,"%d %%", 
    vdgui->engine.fuel.trim.shortTerm.bank2
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_engine_load,
    int,
    SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MIN,SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MAX,SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_ERROR,
    "%d %%", vdgui->engine.load
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_vehicle_speed,
    int,
    SAEJ1979_DATA_VEHICULE_SPEED_MIN,SAEJ1979_DATA_VEHICULE_SPEED_MAX,SAEJ1979_DATA_VEHICULE_SPEED_ERROR,"%d km/h", 
    vdgui->engine.vehicleSpeed
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_ethanol_fuel_percent,
    int,
    SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MIN,SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MAX,SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_ERROR,
    "%d %%", vdgui->engine.fuel.ethanol
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_frp_relative,
    double,
    SAEJ1979_DATA_FRP_RELATIVE_MIN,SAEJ1979_DATA_FRP_RELATIVE_MAX,SAEJ1979_DATA_FRP_RELATIVE_ERROR,
    "%.2f kPa", vdgui->engine.fuel.rail.pressure
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_fuel_injection_timing,
    double,
    SAEJ1979_DATA_FUEL_INJECTION_TIMING_MIN,SAEJ1979_DATA_FUEL_INJECTION_TIMING_MAX,SAEJ1979_DATA_FUEL_INJECTION_TIMING_ERROR,
    "%.2f °", vdgui->engine.injectionSystem.injectionTiming
)
VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_timing_advance_cycle_1,
    double,
    SAEJ1979_DATA_TIMING_ADVANCE_CYCLE_1_MIN,SAEJ1979_DATA_TIMING_ADVANCE_CYCLE_1_MAX,SAEJ1979_DATA_TIMING_ADVANCE_CYCLE_1_ERROR,
    "%.2f °", vdgui->engine.injectionSystem.timingAdvance
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_engine_fuel_rate,
    double,
    SAEJ1979_DATA_ENGINE_FUEL_RATE_MIN,SAEJ1979_DATA_ENGINE_FUEL_RATE_MAX,SAEJ1979_DATA_ENGINE_FUEL_RATE_ERROR,
    "%.2f L/h", vdgui->engine.fuel.rate
)

gboolean vehicle_explorer_saej1979_data_ecu_voltage_gsource(gpointer data) {
    double *currentECUvoltage = data;
    if ( *currentECUvoltage != SAEJ1979_DATA_ECU_VOLTAGE_ERROR ) {
        gtk_label_printf(vdgui->engine.ecu.voltage,
            "%.3f V", *currentECUvoltage
        );
    }    
    free(data);
    return false;
}

gboolean vehicle_explorer_saej1979_data_seconds_since_engine_start_gsource(gpointer data) {
    int *secondsSinceStart = data;
    if ( *secondsSinceStart != SAEJ1979_DATA_SECONDS_SINCE_ENGINE_START_ERROR ) {
        gtk_label_printf(vdgui->engine.secondsSinceStart,
            "%d secs", *secondsSinceStart
        );
    }    
    free(data);
    return false;
}

#define VH_REFRESH_WIDGET(w,data_gen,type) \
    if ( VH_SHOULD_REFRESH_WIDGET(w) ) { \
        g_idle_add(vehicle_explorer_##data_gen##_gsource,type##dup(data_gen(iface, vehicle_explorer_show_freeze_frame_get_state()))); \
    }

gboolean vehicle_explorer_saej1979_data_fuel_system_status_gsource(gpointer data) {
    char *status = data;
    if ( status != null ) {
        gtk_label_printf(vdgui->engine.fuel.status,
            "%s", status
        );
    }    
    free(data);
    return false;
}

#define VH_OX_SENSOR_GSOURCE_SYM(sensor_i) \
    gboolean vehicle_explorer_saej1979_data_oxygen_sensor_voltage_##sensor_i##_gsource(gpointer data) { \
        double *volts = data; \
        if ( volts != null ) { \
            gtk_label_printf(vdgui->engine.oxSensors.sensor_##sensor_i.voltage, \
                "%.2f V", *volts \
            );        \
        } \
        free(data); \
        return false; \
    } \
    gboolean vehicle_explorer_saej1979_data_oxygen_sensor_current_##sensor_i##_gsource(gpointer data) { \
        int *current = data; \
        if ( current != null ) { \
            gtk_label_printf(vdgui->engine.oxSensors.sensor_##sensor_i.current, \
                "%d mA", *current \
            );        \
        } \
        free(data); \
        return false; \
    } \
    gboolean vehicle_explorer_saej1979_data_oxygen_sensor_air_fuel_equiv_ratio_##sensor_i##_gsource(gpointer data) { \
        double *ratio = data; \
        if ( ratio != null ) { \
            gtk_label_printf(vdgui->engine.oxSensors.sensor_##sensor_i.ratio, \
                "%.2f ratio", *ratio \
            );        \
        } \
        free(data); \
        return false; \
    }    

VH_OX_SENSOR_GSOURCE_SYM(1) VH_OX_SENSOR_GSOURCE_SYM(2) VH_OX_SENSOR_GSOURCE_SYM(3)
VH_OX_SENSOR_GSOURCE_SYM(4) VH_OX_SENSOR_GSOURCE_SYM(5) VH_OX_SENSOR_GSOURCE_SYM(6)
VH_OX_SENSOR_GSOURCE_SYM(7) VH_OX_SENSOR_GSOURCE_SYM(8)

#define VH_REFRESH_OX_SENSOR(sensor_i) \
    if ( VH_SHOULD_REFRESH_WIDGET(vdgui->engine.oxSensors.sensor_##sensor_i.voltage) ) { \
        g_idle_add(vehicle_explorer_saej1979_data_oxygen_sensor_voltage_##sensor_i##_gsource,doubledup(saej1979_data_oxygen_sensor_voltage(iface, vehicle_explorer_show_freeze_frame_get_state(), sensor_i))); \
    } \
    if ( VH_SHOULD_REFRESH_WIDGET(vdgui->engine.oxSensors.sensor_##sensor_i.current) ) { \
        g_idle_add(vehicle_explorer_saej1979_data_oxygen_sensor_current_##sensor_i##_gsource,intdup(saej1979_data_oxygen_sensor_current(iface, vehicle_explorer_show_freeze_frame_get_state(), sensor_i))); \
    } \
    if ( VH_SHOULD_REFRESH_WIDGET(vdgui->engine.oxSensors.sensor_##sensor_i.ratio) ) { \
        g_idle_add(vehicle_explorer_saej1979_data_oxygen_sensor_air_fuel_equiv_ratio_##sensor_i##_gsource,doubledup(saej1979_data_oxygen_sensor_air_fuel_equiv_ratio(iface, vehicle_explorer_show_freeze_frame_get_state(), sensor_i))); \
    }

gboolean vehicle_explorer_saej1979_data_tests_gsource(gpointer data) {
    GList* glist = gtk_container_get_children(vdgui->engine.tests);
    while(glist != null ) {
        gtk_container_remove(vdgui->engine.tests, glist->data);
        glist = glist->next;
    }
    SAEJ1979_DATA_Test_list *testsList = data;
    for(int i = 0; i < testsList->size; i ++) {
        SAEJ1979_DATA_Test * test = testsList->list[i];
        GtkBox * box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5));
        gtk_container_add(GTK_CONTAINER(box), gtk_label_new(test->name));
        gtk_container_add(GTK_CONTAINER(box), gtk_label_new(test->completed ? "complete" : "incomplete"));
        gtk_container_add(vdgui->engine.tests, GTK_WIDGET(box));
        gtk_widget_show_all(GTK_WIDGET(box));
        free(test->name);
    }    
    SAEJ1979_DATA_Test_list_free(data);
    return false;        
}

bool vehicle_explorer_refresh_dynamic_internal() {
    final OBDIFace* iface = config.ephemere.iface;
    if ( vehicle_explorer_error_feedback_obd(iface) ) {
        return false;
    } else {
        bool useFreezeFrame = vehicle_explorer_show_freeze_frame_get_state();
        VH_REFRESH_WIDGET(vdgui->engine.coolant.temperature,                        saej1979_data_engine_coolant_temperature,   int);
        VH_REFRESH_WIDGET(vdgui->engine.intakeAir.temperature,                      saej1979_data_intake_air_temperature,       int);
        VH_REFRESH_WIDGET(vdgui->engine.intakeAir.manifoldPressure,                 saej1979_data_intake_manifold_pressure,     int);
        VH_REFRESH_WIDGET(vdgui->engine.intakeAir.mafRate,                          saej1979_data_maf_air_flow_rate,            double);        
        VH_REFRESH_WIDGET(vdgui->engine.speed,                                      saej1979_data_engine_speed,                 double);
        VH_REFRESH_WIDGET(vdgui->engine.ecu.voltage,                                saej1979_data_ecu_voltage,                  double);
        VH_REFRESH_WIDGET(vdgui->engine.load,                                       saej1979_data_engine_load,                  int);               
        VH_REFRESH_WIDGET(vdgui->engine.vehicleSpeed,                               saej1979_data_vehicle_speed,                int);
        VH_REFRESH_WIDGET(vdgui->engine.secondsSinceStart,                          saej1979_data_seconds_since_engine_start,   int);
        VH_REFRESH_WIDGET(vdgui->engine.fuel.pressure,                              saej1979_data_fuel_pressure,                int);
        VH_REFRESH_WIDGET(vdgui->engine.fuel.level,                                 saej1979_data_fuel_tank_level_input,        double);
        VH_REFRESH_WIDGET(vdgui->engine.fuel.ethanol,                               saej1979_data_ethanol_fuel_percent,         int);
        VH_REFRESH_WIDGET(vdgui->engine.fuel.rail.pressure,                         saej1979_data_frp_relative,                 double);
        VH_REFRESH_WIDGET(vdgui->engine.fuel.rate,                                  saej1979_data_engine_fuel_rate,             double);        
        VH_REFRESH_WIDGET(vdgui->engine.fuel.trim.longTerm.bank1,                   saej1979_data_long_term_fuel_trim_bank_1,   int);
        VH_REFRESH_WIDGET(vdgui->engine.fuel.trim.longTerm.bank2,                   saej1979_data_long_term_fuel_trim_bank_2,   int);  
        VH_REFRESH_WIDGET(vdgui->engine.fuel.trim.shortTerm.bank1,                  saej1979_data_short_term_fuel_trim_bank_1,  int);
        VH_REFRESH_WIDGET(vdgui->engine.fuel.trim.shortTerm.bank2,                  saej1979_data_short_term_fuel_trim_bank_2,  int);               
        VH_REFRESH_WIDGET(vdgui->engine.injectionSystem.injectionTiming,            saej1979_data_fuel_injection_timing,        double);        
        VH_REFRESH_WIDGET(vdgui->engine.injectionSystem.timingAdvance,              saej1979_data_timing_advance_cycle_1,       double);        
        VH_REFRESH_OX_SENSOR(1) VH_REFRESH_OX_SENSOR(2) VH_REFRESH_OX_SENSOR(3) VH_REFRESH_OX_SENSOR(4)
        VH_REFRESH_OX_SENSOR(5) VH_REFRESH_OX_SENSOR(6) VH_REFRESH_OX_SENSOR(7) VH_REFRESH_OX_SENSOR(8)

        if ( VH_SHOULD_REFRESH_WIDGET(gtk_widget_get_parent(GTK_WIDGET(vdgui->engine.tests))) ) {
            SAEJ1979_DATA_Test_list *testsList = saej1979_data_tests(iface, useFreezeFrame, false);
            if ( testsList != null ) {
                g_idle_add(vehicle_explorer_saej1979_data_tests_gsource,testsList);
            }
        }
        
        if ( VH_SHOULD_REFRESH_WIDGET(vdgui->engine.fuel.status) ) {
            char ** status = saej1979_data_fuel_system_status(iface, useFreezeFrame);
            if ( status != null ) {
                g_idle_add(vehicle_explorer_saej1979_data_fuel_system_status_gsource,status[0]);
            }
        }
    }
    return true;
}

void* vehicle_explorer_refresh_dynamic_daemon(void *arg) {
    pthread_cleanup_push(obd_thread_cleanup_routine, null);
    const struct timespec req = {
        .tv_sec = config.vehicleExplorer.refreshRateS,
        .tv_nsec = (config.vehicleExplorer.refreshRateS - ((int)config.vehicleExplorer.refreshRateS)) * 1000000000
    };
    while(vehicle_explorer_refresh_dynamic_internal()) {
        if ( nanosleep(&req,null) == -1 ) {
            break;
        }
    }
    pthread_cleanup_pop(1);
}

void vehicle_explorer_refresh_dynamic() {
    if ( vehicle_explorer_refresh_dynamic_thread == null ) {
        vehicle_explorer_refresh_dynamic_thread = (pthread_t*)malloc(sizeof(pthread_t));
        if ( pthread_create(vehicle_explorer_refresh_dynamic_thread, null, &vehicle_explorer_refresh_dynamic_daemon, null) == 0 ) {
            module_debug(MODULE_VEHICULE_DIAGNOSTIC "Checker thread created");
        } else {
            module_debug(MODULE_VEHICULE_DIAGNOSTIC "Fatal erro during thead creation");
        }
    }
}

#define VH_GTK_LABEL_SET_TEXT_SYM(sym,label) \
    gboolean vehicle_explorer_##sym##_gsource(gpointer data) { \
        char *value = data; \
        gtk_label_set_text(label, value); \
        free(data); \
        return false; \
    }

#define VH_REFRESH_LABEL(sym) \
    g_idle_add(vehicle_explorer_##sym##_gsource,sym(iface, vehicle_explorer_show_freeze_frame_get_state()))

VH_GTK_LABEL_SET_TEXT_SYM(saej1979_data_engine_type_as_string, vdgui->engine.type)
VH_GTK_LABEL_SET_TEXT_SYM(saej1979_data_obd_standard_as_string, vdgui->engine.ecu.obdStandard)
VH_GTK_LABEL_SET_TEXT_SYM(saej1979_data_fuel_type_as_string, vdgui->engine.fuel.type)


void vehicle_explorer_refresh_one_time_internal() {
    final OBDIFace* iface = config.ephemere.iface;
    if ( ! vehicle_explorer_error_feedback_obd(iface) ) {
        VH_REFRESH_LABEL(saej1979_data_engine_type_as_string);
        VH_REFRESH_LABEL(saej1979_data_obd_standard_as_string);
        VH_REFRESH_LABEL(saej1979_data_fuel_type_as_string);
    }
}

void* vehicle_explorer_refresh_one_time_daemon(void *arg) {
    vehicle_explorer_refresh_dynamic_internal(); 
    vehicle_explorer_refresh_one_time_internal(); 
}

void vehicle_explorer_refresh_one_time() {
    pthread_t t;
    pthread_create(&t, null, &vehicle_explorer_refresh_one_time_daemon, null);
}

void* vehicle_explorer_refresh_one_time_with_spinner_daemon(final void *arg) {
    final pthread_t * t = (pthread_t*)arg;
    gtk_spinner_start(vdgui->refreshIcon);
    pthread_join(*t, NULL);
    gtk_spinner_stop(vdgui->refreshIcon);
    free(t);
}
void vehicle_explorer_refresh_one_time_with_spinner() {
    pthread_t *t,t1;
    t = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(t, null, &vehicle_explorer_refresh_one_time_daemon, null);
    pthread_create(&t1, null, &vehicle_explorer_refresh_one_time_with_spinner_daemon, t);
}
void vehicle_explorer_refresh_changed (GtkCheckMenuItem *checkmenuitem, gpointer user_data) {
    bool state = gtk_check_menu_item_get_active(checkmenuitem);
    config.vehicleExplorer.autoRefresh = state;
    config_store();
    if ( state ) {
        vehicle_explorer_refresh_dynamic();        
    } else {
        THREAD_CANCEL(vehicle_explorer_refresh_dynamic_thread);    
    }
}
void vehicle_explorer_show_window() {
    gtk_widget_show_now (vdgui->window);
    gtk_check_menu_item_set_active(vdgui->menuBar.autoRefresh,config.vehicleExplorer.autoRefresh);
    vehicle_explorer_refresh_one_time();
    vehicle_explorer_refresh_changed(vdgui->menuBar.autoRefresh,null);
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
    } else {
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
    if ( strcmp(name,"GtkExpander") == 0 ) {
        g_signal_connect(G_OBJECT(widget),"activate",G_CALLBACK(vehicle_explorer_mapped_true_one_level),NULL);
        gtk_container_foreach(
            GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(widget))),
            vehicle_explorer_mapped_false,&mapped
            );
    } else if ( strcmp(name,"GtkBox") == 0 ) {
        gtk_container_foreach(GTK_CONTAINER(widget),
            vehicle_explorer_mapped_false, &mapped);
    } else {
        gtk_widget_set_mapped(widget,mapped);
        gtk_widget_set_visible(widget,mapped);        
    }
}

#define VH_RETRIEVE_OX_SENSOR(i) \
    .sensor_##i = { \
        .voltage = GTK_LABEL(gtk_builder_get_object (builder, "vehicle-explorer-engine-sensors-"#i"-voltage")), \
        .current = GTK_LABEL(gtk_builder_get_object (builder, "vehicle-explorer-engine-sensors-"#i"-current")), \
        .ratio = GTK_LABEL(gtk_builder_get_object (builder, "vehicle-explorer-engine-sensors-"#i"-ratio")), \
    }

void module_init_vehicle_explorer(final GtkBuilder *builder) {
    if ( vdgui == null ) {
        vehicleExplorerGui g = {
            .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-vehicle-explorer")),
            .refreshIcon = (GtkSpinner*)gtk_builder_get_object (builder, "window-vehicle-explorer-global-refresh"),
            .errorFeedback = ERROR_FEEDBACK_RETRIEVE_WINDOWS(builder),
            .menuBar = {
                .autoRefresh = (GtkCheckMenuItem*)gtk_builder_get_object(builder,"vehicle-explorer-menubar-autorefresh"),
                .freeze_frame_thread = null,
                .freeze_frame_error_popup = GTK_WINDOW(gtk_builder_get_object(builder,"vehicle-explorer-freeze-frame-error")),
                .showFreezeFrame = GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder,"vehicle-explorer-menubar-show-freezeframe"))
            },
            .engine = {
                .load = GTK_PROGRESS_BAR(gtk_builder_get_object(builder,"vehicle-explorer-engine-load")),
                .expandableSection = (GtkBox *)gtk_builder_get_object(builder,"vehicle-explorer-expandable-section"),
                .speed = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-speed-graph"),
                .type = (GtkLabel*)gtk_builder_get_object (builder, "vehicle-explorer-engine-type"),
                .vehicleSpeed = GTK_PROGRESS_BAR(gtk_builder_get_object(builder,"vehicle-explorer-vehicle-speed")),
                .secondsSinceStart = GTK_LABEL(gtk_builder_get_object(builder,"vehicle-explorer-engine-seconds-since-start")),
                .intakeAir = {
                    .temperature = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-intake-air-temperature"),
                    .manifoldPressure = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-intake-air-manifold-pressure"),
                    .mafRate = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-intake-air-maf-rate")
                },
                .fuel = {
                    .pressure = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-pressure"),
                    .level = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-level"),
                    .type = GTK_LABEL(gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-type")),
                    .ethanol = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-ethanol")),
                    .rate = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-rate")),                    
                    .trim = {
                        .longTerm = {
                            .bank1 = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-trim-long-term-1"),
                            .bank2 = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-trim-long-term-2")
                        },
                        .shortTerm = {
                            .bank1 = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-trim-short-term-1"),
                            .bank2 = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-engine-fuel-trim-short-term-2")
                        }
                    },
                    .rail = {
                        .pressure = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-rail-pressure"))
                    },
                    .status = GTK_LABEL(gtk_builder_get_object (builder,"vehicle-explorer-engine-fuel-system-status"))
                },
                .injectionSystem = {
                    .injectionTiming = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-injection-timing")),
                    .timingAdvance = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "vehicle-explorer-engine-time-advance"))
                },
                .coolant = {
                    .temperature = (GtkProgressBar *)gtk_builder_get_object (builder, "vehicle-explorer-coolant-temperature-graph")
                },
                .ecu = {
                    .voltage = (GtkLabel*)gtk_builder_get_object (builder, "vehicle-explorer-ecu-voltage"),
                    .obdStandard = (GtkLabel*)gtk_builder_get_object (builder, "vehicle-explorer-ecu-obd-standard")
                },
                .oxSensors = {
                    VH_RETRIEVE_OX_SENSOR(1),VH_RETRIEVE_OX_SENSOR(2),VH_RETRIEVE_OX_SENSOR(3),VH_RETRIEVE_OX_SENSOR(4),
                    VH_RETRIEVE_OX_SENSOR(5),VH_RETRIEVE_OX_SENSOR(6),VH_RETRIEVE_OX_SENSOR(7),VH_RETRIEVE_OX_SENSOR(8)
                },
                .tests = GTK_CONTAINER(gtk_builder_get_object (builder,"vehicle-explorer-engine-tests"))
            }
        };
        
        vdgui = (vehicleExplorerGui*)malloc(sizeof(vehicleExplorerGui));
        (*vdgui) = g;
        counter_init_with(g.engine.vehicleSpeed,"counter_85_2_255_0_0_255.png");
        counter_init_with(g.engine.speed,"counter_85_2_255_0_0_255.png");
        counter_init_with(g.engine.fuel.pressure,"gaugehalf_225_5_255_0_0_255.png");
        counter_init_with(g.engine.fuel.rail.pressure,"gaugehalf_225_5_255_0_0_255.png");        
        vehicle_explorer_mapped_false((GtkWidget*)vdgui->engine.expandableSection,null);
        g_signal_connect(G_OBJECT(vdgui->window),"delete-event",G_CALLBACK(vehicle_explorer_onclose),NULL);
        g_signal_connect(G_OBJECT(g.menuBar.freeze_frame_error_popup),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        error_feedback_windows_init(vdgui->errorFeedback);
        gtk_builder_add_callback_symbol(builder,"show-window-vehicle-explorer",&vehicle_explorer_show_window);
        gtk_builder_add_callback_symbol(builder,"window-vehicle-explorer-global-refresh-click",&vehicle_explorer_refresh_one_time_with_spinner);
        gtk_builder_add_callback_symbol(builder,"window-vehicle-explorer-global-refresh-changed",G_CALLBACK(&vehicle_explorer_refresh_changed));
        gtk_builder_add_callback_symbol(builder,"vehicle-explorer-freeze-frame-error-ok",G_CALLBACK(&vehicle_explorer_freeze_frame_error_ok));
        gtk_builder_add_callback_symbol(builder,"window-vehicle-explorer-data-freeze-frame",&vehicle_explorer_data_freeze_frame);
    } else {
        module_debug(MODULE_VEHICULE_DIAGNOSTIC "module already initialized");
    }
}


