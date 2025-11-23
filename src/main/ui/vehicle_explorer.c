#include "ui/vehicle_explorer.h"

static vehicleExplorerGui *gui = null;
static pthread_t *refresh_dynamic_thread = null;
static list_Graph *graphs = null;
static pthread_mutex_t graphs_mutex;

static void button_click_clean_up_routine(void *arg) {
    obd_thread_cleanup_routine(arg);
    gtk_spinner_stop(gui->refreshIcon);
}

static bool show_freeze_frame_get_state() {
    return gtk_check_menu_item_get_active(gui->menuBar.showFreezeFrame);    
}

static void freeze_frame_error_ok() {
    gtk_widget_hide_on_main_thread(GTK_WIDGET(gui->menuBar.freeze_frame_error_popup));
}
static bool error_feedback(GtkMessageDialog * widget, char * title, char *msg) {
    if ( msg == null && title == null ) {
        return false;
    } else {
        if ( msg == null ) {
            msg = strdup(" ");
        }
        if ( title != null ) {
            gtk_window_set_title(GTK_WINDOW(widget), title);
        }
        gtk_message_dialog_set_markup(widget, msg);
        gtk_widget_show_on_main_thread(GTK_WIDGET(widget));
        return true;
    }
}
static void data_freeze_frame_internal() {
    final VehicleIFace* iface = config.ephemere.iface;
    if ( vehicle_explorer_error_feedback_obd(iface) ) {
        return;
    } else {
        gtk_spinner_start(gui->refreshIcon);
        if ( ! saej1979_data_freeze_frame(iface) ) {
            gtk_widget_show_on_main_thread(GTK_WIDGET(gui->menuBar.freeze_frame_error_popup));
        }
    }
}
THREAD_WRITE_DAEMON(
    data_freeze_frame_daemon,
    data_freeze_frame_internal,
    button_click_clean_up_routine, gui->menuBar.freeze_frame_thread
)

static void data_freeze_frame() {
    thread_allocate_and_start(&gui->menuBar.freeze_frame_thread,&data_freeze_frame_daemon);    
}

static gboolean onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused) {
   module_debug(MODULE_VEHICLE_DIAGNOSTIC "Close event received");
   THREAD_CANCEL(refresh_dynamic_thread);
   gtk_widget_hide(gui->window);
   return true;
}

#define VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(sym,type,min,max,error,format,bar) \
    static GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(sym##_gsource,type,min,max,error,format,bar)

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
    "%d °C", gui->engine.coolant.temperature
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_engine_speed,
    double,
    SAEJ1979_DATA_ENGINE_SPEED_MIN,SAEJ1979_DATA_ENGINE_SPEED_MAX,SAEJ1979_DATA_ENGINE_SPEED_ERROR,
    "%.2f r/min", gui->engine.speed
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_fuel_pressure,
    int,
    SAEJ1979_DATA_FUEL_PRESSURE_MIN,SAEJ1979_DATA_FUEL_PRESSURE_MAX,SAEJ1979_DATA_FUEL_PRESSURE_ERROR,
    "%d kPa", gui->engine.fuel.pressure
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_fuel_tank_level_input,
    double,
    SAEJ1979_DATA_FUEL_TANK_LEVEL_INPUT_MIN,SAEJ1979_DATA_FUEL_TANK_LEVEL_INPUT_MAX,
    SAEJ1979_DATA_FUEL_TANK_LEVEL_INPUT_ERROR, "%.2f %%", gui->engine.fuel.level
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_intake_air_temperature,
    int,
    SAEJ1979_DATA_ENGINE_INTAKE_AIR_TEMPERATURE_MIN,SAEJ1979_DATA_ENGINE_INTAKE_AIR_TEMPERATURE_MAX,SAEJ1979_DATA_ENGINE_INTAKE_AIR_TEMPERATURE_ERROR,
    "%d °C", gui->engine.intakeAir.temperature
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_intake_manifold_pressure,
    int,
    SAEJ1979_DATA_INTAKE_MANIFOLD_PRESSURE_MIN,SAEJ1979_DATA_INTAKE_MANIFOLD_PRESSURE_MAX,SAEJ1979_DATA_INTAKE_MANIFOLD_PRESSURE_ERROR,
    "%d kPa", gui->engine.intakeAir.manifoldPressure
)
VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_maf_air_flow_rate,
    double,
    SAEJ1979_DATA_VEHICLE_MAF_AIR_FLOW_RATE_MIN,SAEJ1979_DATA_VEHICLE_MAF_AIR_FLOW_RATE_MAX,SAEJ1979_DATA_VEHICLE_MAF_AIR_FLOW_RATE_ERROR,
    "%.2f g/s", gui->engine.intakeAir.mafRate
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_long_term_fuel_trim_bank_1,
    double,
    SAEJ1979_DATA_FUEL_TRIM_MIN,SAEJ1979_DATA_FUEL_TRIM_MAX,SAEJ1979_DATA_FUEL_TRIM_ERROR,"%.2f %%", 
    gui->engine.fuel.trim.longTerm.bank1
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_long_term_fuel_trim_bank_2,
    double,
    SAEJ1979_DATA_FUEL_TRIM_MIN,SAEJ1979_DATA_FUEL_TRIM_MAX,SAEJ1979_DATA_FUEL_TRIM_ERROR,"%.2f %%", 
    gui->engine.fuel.trim.longTerm.bank2
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_short_term_fuel_trim_bank_1,
    double,
    SAEJ1979_DATA_FUEL_TRIM_MIN,SAEJ1979_DATA_FUEL_TRIM_MAX,SAEJ1979_DATA_FUEL_TRIM_ERROR,"%.2f %%", 
    gui->engine.fuel.trim.shortTerm.bank1
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_short_term_fuel_trim_bank_2,
    double,
    SAEJ1979_DATA_FUEL_TRIM_MIN,SAEJ1979_DATA_FUEL_TRIM_MAX,SAEJ1979_DATA_FUEL_TRIM_ERROR,"%.2f %%", 
    gui->engine.fuel.trim.shortTerm.bank2
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_engine_load,
    int,
    SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MIN,SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MAX,SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_ERROR,
    "%d %%", gui->engine.load
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_vehicle_speed,
    int,
    SAEJ1979_DATA_VEHICLE_SPEED_MIN,SAEJ1979_DATA_VEHICLE_SPEED_MAX,SAEJ1979_DATA_VEHICLE_SPEED_ERROR,"%d km/h", 
    gui->engine.vehicleSpeed
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_ethanol_fuel_percent,
    int,
    SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MIN,SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MAX,SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_ERROR,
    "%d %%", gui->engine.fuel.ethanol
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_frp_relative,
    double,
    SAEJ1979_DATA_FRP_RELATIVE_MIN,SAEJ1979_DATA_FRP_RELATIVE_MAX,SAEJ1979_DATA_FRP_RELATIVE_ERROR,
    "%.2f kPa", gui->engine.fuel.rail.pressure
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_fuel_injection_timing,
    double,
    SAEJ1979_DATA_FUEL_INJECTION_TIMING_MIN,SAEJ1979_DATA_FUEL_INJECTION_TIMING_MAX,SAEJ1979_DATA_FUEL_INJECTION_TIMING_ERROR,
    "%.2f °", gui->engine.injectionSystem.injectionTiming
)
VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_timing_advance_cycle_1,
    double,
    SAEJ1979_DATA_TIMING_ADVANCE_CYCLE_1_MIN,SAEJ1979_DATA_TIMING_ADVANCE_CYCLE_1_MAX,SAEJ1979_DATA_TIMING_ADVANCE_CYCLE_1_ERROR,
    "%.2f °", gui->engine.injectionSystem.timingAdvance
)

VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(saej1979_data_engine_fuel_rate,
    double,
    SAEJ1979_DATA_ENGINE_FUEL_RATE_MIN,SAEJ1979_DATA_ENGINE_FUEL_RATE_MAX,SAEJ1979_DATA_ENGINE_FUEL_RATE_ERROR,
    "%.2f L/h", gui->engine.fuel.rate
)

static gboolean saej1979_data_ecu_voltage_gsource(gpointer data) {
    double *currentECUvoltage = data;
    if ( *currentECUvoltage != SAEJ1979_DATA_ECU_VOLTAGE_ERROR ) {
        gtk_label_printf(gui->engine.ecu.voltage,
            "%.3f V", *currentECUvoltage
        );
    }    
    free(data);
    return false;
}

static gboolean saej1979_data_seconds_since_engine_start_gsource(gpointer data) {
    int *secondsSinceStart = data;
    if ( *secondsSinceStart != SAEJ1979_DATA_SECONDS_SINCE_ENGINE_START_ERROR ) {
        gtk_label_printf(gui->engine.secondsSinceStart,
            "%d secs", *secondsSinceStart
        );
    }    
    free(data);
    return false;
}

#define VH_REFRESH_WIDGET(w,data_gen,type) \
    if ( VH_SHOULD_REFRESH_WIDGET(w) ) { \
        g_idle_add(data_gen##_gsource,type##dup(data_gen(iface, show_freeze_frame_get_state()))); \
    }

#define VH_REFRESH_GRAPH_GSOURCE_SYM(data_gen) gboolean graph_##data_gen##_gsource(gpointer data)
VH_REFRESH_GRAPH_GSOURCE_SYM(refresh) {
    final Graph * graph = (Graph*)data;
    gtk_widget_queue_draw(graph->widget);
    return false;
}
#define VH_REFRESH_GRAPH(data_gen, data_gen_error, graphType, ...) { \
    Graph * graph = list_Graph_get_by_title(graphs, graphType); \
    if ( graph != null ) { \
        if ( VH_SHOULD_REFRESH_WIDGET(graph->widget) ) { \
            double value = data_gen(iface, show_freeze_frame_get_state(), ##__VA_ARGS__); \
            if ( value != data_gen_error ) { \
                list_Graph_append_data(graphs, graphType, value); \
                g_idle_add(graph_refresh_gsource,graph); \
            } \
        } \
    } \
}

static gboolean saej1979_data_fuel_system_status_gsource(gpointer data) {
    char *status = data;
    if ( status != null ) {
        gtk_label_printf(gui->engine.fuel.status,
            "%s", status
        );
    }    
    free(data);
    return false;
}

#define VH_OX_SENSOR_GSOURCE_SYM(sensor_i) \
    static gboolean saej1979_data_oxygen_sensor_voltage_##sensor_i##_gsource(gpointer data) { \
        double *volts = data; \
        if ( volts != null ) { \
            gtk_label_printf(gui->engine.oxSensors.sensor_##sensor_i.voltage, \
                "%.2f V", *volts \
            );        \
        } \
        free(data); \
        return false; \
    } \
    static gboolean saej1979_data_oxygen_sensor_current_##sensor_i##_gsource(gpointer data) { \
        int *current = data; \
        if ( current != null ) { \
            gtk_label_printf(gui->engine.oxSensors.sensor_##sensor_i.current, \
                "%d mA", *current \
            );        \
        } \
        free(data); \
        return false; \
    } \
    static gboolean saej1979_data_oxygen_sensor_air_fuel_equiv_ratio_##sensor_i##_gsource(gpointer data) { \
        double *ratio = data; \
        if ( ratio != null ) { \
            gtk_label_printf(gui->engine.oxSensors.sensor_##sensor_i.ratio, \
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
    if ( VH_SHOULD_REFRESH_WIDGET(gui->engine.oxSensors.sensor_##sensor_i.voltage) ) { \
        g_idle_add(saej1979_data_oxygen_sensor_voltage_##sensor_i##_gsource,doubledup(saej1979_data_oxygen_sensor_voltage(iface, show_freeze_frame_get_state(), sensor_i))); \
    } \
    if ( VH_SHOULD_REFRESH_WIDGET(gui->engine.oxSensors.sensor_##sensor_i.current) ) { \
        g_idle_add(saej1979_data_oxygen_sensor_current_##sensor_i##_gsource,intdup(saej1979_data_oxygen_sensor_current(iface, show_freeze_frame_get_state(), sensor_i))); \
    } \
    if ( VH_SHOULD_REFRESH_WIDGET(gui->engine.oxSensors.sensor_##sensor_i.ratio) ) { \
        g_idle_add(saej1979_data_oxygen_sensor_air_fuel_equiv_ratio_##sensor_i##_gsource,doubledup(saej1979_data_oxygen_sensor_air_fuel_equiv_ratio(iface, show_freeze_frame_get_state(), sensor_i))); \
    }

#define VH_REFRESH_GRAPH_OX_SENSORS() { \
    for(int sensor_i = 1; sensor_i <= 8; sensor_i++) { \
        char graphTitle[64]; \
        snprintf(graphTitle, sizeof(graphTitle), "Oxygen sensor %d voltage", sensor_i); \
        VH_REFRESH_GRAPH( \
            saej1979_data_oxygen_sensor_voltage, \
            SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_ERROR, \
            graphTitle, sensor_i \
        ) \
        snprintf(graphTitle, sizeof(graphTitle), "Oxygen sensor %d current", sensor_i); \
        VH_REFRESH_GRAPH( \
            saej1979_data_oxygen_sensor_current, \
            SAEJ1979_DATA_OXYGEN_SENSOR_CURRENT_ERROR, \
            graphTitle, sensor_i \
        ) \
        snprintf(graphTitle, sizeof(graphTitle), "Oxygen sensor %d air fuel equivalence ratio", sensor_i); \
        VH_REFRESH_GRAPH( \
            saej1979_data_oxygen_sensor_air_fuel_equiv_ratio, \
            SAEJ1979_DATA_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_ERROR, \
            graphTitle, sensor_i \
        ) \
    } \
}

static gboolean saej1979_data_tests_gsource(gpointer data) {
    GList* glist = gtk_container_get_children(gui->engine.tests);
    while(glist != null ) {
        gtk_container_remove(gui->engine.tests, glist->data);
        glist = glist->next;
    }
    list_SAEJ1979_DATA_Test *testsList = data;
    for(int i = 0; i < testsList->size; i ++) {
        SAEJ1979_DATA_Test * test = testsList->list[i];
        GtkBox * box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5));
        gtk_container_add(GTK_CONTAINER(box), gtk_label_new(test->name));
        gtk_container_add(GTK_CONTAINER(box), gtk_label_new(test->completed ? "complete" : "incomplete"));
        gtk_container_add(gui->engine.tests, GTK_WIDGET(box));
        gtk_widget_show_all(GTK_WIDGET(box));
        free(test->name);
    }    
    list_SAEJ1979_DATA_Test_free(data);
    return false;        
}

static bool refresh_dynamic_internal() {
    final VehicleIFace* iface = config.ephemere.iface;
    if ( vehicle_explorer_error_feedback_obd(iface) ) {
        return false;
    } else {
        bool useFreezeFrame = show_freeze_frame_get_state();
        VH_REFRESH_WIDGET(gui->engine.coolant.temperature,                        saej1979_data_engine_coolant_temperature,   int);
        VH_REFRESH_WIDGET(gui->engine.intakeAir.temperature,                      saej1979_data_intake_air_temperature,       int);
        VH_REFRESH_WIDGET(gui->engine.intakeAir.manifoldPressure,                 saej1979_data_intake_manifold_pressure,     int);
        VH_REFRESH_WIDGET(gui->engine.intakeAir.mafRate,                          saej1979_data_maf_air_flow_rate,            double);        
        VH_REFRESH_WIDGET(gui->engine.speed,                                      saej1979_data_engine_speed,                 double);
        VH_REFRESH_WIDGET(gui->engine.ecu.voltage,                                saej1979_data_ecu_voltage,                  double);
        VH_REFRESH_WIDGET(gui->engine.load,                                       saej1979_data_engine_load,                  int);               
        VH_REFRESH_WIDGET(gui->engine.vehicleSpeed,                               saej1979_data_vehicle_speed,                int);
        VH_REFRESH_WIDGET(gui->engine.secondsSinceStart,                          saej1979_data_seconds_since_engine_start,   int);
        VH_REFRESH_WIDGET(gui->engine.fuel.pressure,                              saej1979_data_fuel_pressure,                int);
        VH_REFRESH_WIDGET(gui->engine.fuel.level,                                 saej1979_data_fuel_tank_level_input,        double);
        VH_REFRESH_WIDGET(gui->engine.fuel.ethanol,                               saej1979_data_ethanol_fuel_percent,         int);
        VH_REFRESH_WIDGET(gui->engine.fuel.rail.pressure,                         saej1979_data_frp_relative,                 double);
        VH_REFRESH_WIDGET(gui->engine.fuel.rate,                                  saej1979_data_engine_fuel_rate,             double);        
        VH_REFRESH_WIDGET(gui->engine.fuel.trim.longTerm.bank1,                   saej1979_data_long_term_fuel_trim_bank_1,   double);
        VH_REFRESH_WIDGET(gui->engine.fuel.trim.longTerm.bank2,                   saej1979_data_long_term_fuel_trim_bank_2,   double);  
        VH_REFRESH_WIDGET(gui->engine.fuel.trim.shortTerm.bank1,                  saej1979_data_short_term_fuel_trim_bank_1,  double);
        VH_REFRESH_WIDGET(gui->engine.fuel.trim.shortTerm.bank2,                  saej1979_data_short_term_fuel_trim_bank_2,  double);               
        VH_REFRESH_WIDGET(gui->engine.injectionSystem.injectionTiming,            saej1979_data_fuel_injection_timing,        double);        
        VH_REFRESH_WIDGET(gui->engine.injectionSystem.timingAdvance,              saej1979_data_timing_advance_cycle_1,       double);        
        VH_REFRESH_OX_SENSOR(1) VH_REFRESH_OX_SENSOR(2) VH_REFRESH_OX_SENSOR(3) VH_REFRESH_OX_SENSOR(4)
        VH_REFRESH_OX_SENSOR(5) VH_REFRESH_OX_SENSOR(6) VH_REFRESH_OX_SENSOR(7) VH_REFRESH_OX_SENSOR(8)

        pthread_mutex_lock(&graphs_mutex);
        VH_REFRESH_GRAPH(saej1979_data_engine_coolant_temperature, SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_ERROR, "Coolant Temperature")
        VH_REFRESH_GRAPH(saej1979_data_intake_air_temperature, SAEJ1979_DATA_ENGINE_INTAKE_AIR_TEMPERATURE_ERROR, "Intake Air Temperature")
        VH_REFRESH_GRAPH(saej1979_data_intake_manifold_pressure, SAEJ1979_DATA_INTAKE_MANIFOLD_PRESSURE_ERROR, "Intake Air Manifold Pressure")
        VH_REFRESH_GRAPH(saej1979_data_maf_air_flow_rate, SAEJ1979_DATA_VEHICLE_MAF_AIR_FLOW_RATE_ERROR, "Intake Air MAF Rate")
        VH_REFRESH_GRAPH(saej1979_data_engine_speed, SAEJ1979_DATA_ENGINE_SPEED_ERROR, "Engine Speed")
        VH_REFRESH_GRAPH(saej1979_data_vehicle_speed, SAEJ1979_DATA_VEHICLE_SPEED_ERROR, "Speed")
        VH_REFRESH_GRAPH(saej1979_data_fuel_pressure, SAEJ1979_DATA_FUEL_PRESSURE_ERROR, "Fuel Pressure")
        VH_REFRESH_GRAPH(saej1979_data_fuel_tank_level_input, SAEJ1979_DATA_FUEL_TANK_LEVEL_INPUT_ERROR, "Fuel Level")
        VH_REFRESH_GRAPH(saej1979_data_ethanol_fuel_percent, SAEJ1979_DATA_ETHANOL_FUEL_PERCENT_ERROR, "Fuel ethanol")
        VH_REFRESH_GRAPH(saej1979_data_frp_relative, SAEJ1979_DATA_FRP_RELATIVE_ERROR, "Fuel Rail Pressure")
        VH_REFRESH_GRAPH(saej1979_data_engine_fuel_rate, SAEJ1979_DATA_ENGINE_FUEL_RATE_ERROR, "Fuel rate")
        VH_REFRESH_GRAPH(saej1979_data_long_term_fuel_trim_bank_1, SAEJ1979_DATA_FUEL_TRIM_ERROR, "Fuel trim long term bank1")
        VH_REFRESH_GRAPH(saej1979_data_long_term_fuel_trim_bank_2, SAEJ1979_DATA_FUEL_TRIM_ERROR, "Fuel trim long term bank2")
        VH_REFRESH_GRAPH(saej1979_data_short_term_fuel_trim_bank_1, SAEJ1979_DATA_FUEL_TRIM_ERROR, "Fuel trim short term bank1")
        VH_REFRESH_GRAPH(saej1979_data_short_term_fuel_trim_bank_2, SAEJ1979_DATA_FUEL_TRIM_ERROR, "Fuel trim short term bank2")
        VH_REFRESH_GRAPH(saej1979_data_fuel_injection_timing, SAEJ1979_DATA_FUEL_INJECTION_TIMING_ERROR, "Injection timing")
        VH_REFRESH_GRAPH(saej1979_data_timing_advance_cycle_1, SAEJ1979_DATA_TIMING_ADVANCE_CYCLE_1_ERROR, "Injection timing advance before TDC")
        VH_REFRESH_GRAPH_OX_SENSORS()
        pthread_mutex_unlock(&graphs_mutex);

        if ( VH_SHOULD_REFRESH_WIDGET(gtk_widget_get_parent(GTK_WIDGET(gui->engine.tests))) ) {
            list_SAEJ1979_DATA_Test *testsList = saej1979_data_tests(iface, useFreezeFrame, false);
            if ( testsList != null ) {
                g_idle_add(saej1979_data_tests_gsource,testsList);
            }
        }
        
        if ( VH_SHOULD_REFRESH_WIDGET(gui->engine.fuel.status) ) {
            char ** status = saej1979_data_fuel_system_status(iface, useFreezeFrame);
            if ( status != null ) {
                g_idle_add(saej1979_data_fuel_system_status_gsource,status[0]);
            }
        }
    }
    return true;
}

static void* refresh_dynamic_daemon(void *arg) {
    pthread_cleanup_push(obd_thread_cleanup_routine, null);
    const struct timespec req = {
        .tv_sec = config.vehicleExplorer.refreshRateS,
        .tv_nsec = (config.vehicleExplorer.refreshRateS - ((int)config.vehicleExplorer.refreshRateS)) * 1000000000
    };
    while(refresh_dynamic_internal()) {
        if ( nanosleep(&req,null) == -1 ) {
            break;
        }
    }
    pthread_cleanup_pop(1);
    return null;
}

static void refresh_dynamic() {
    if ( refresh_dynamic_thread == null ) {
        refresh_dynamic_thread = (pthread_t*)malloc(sizeof(pthread_t));
        if ( pthread_create(refresh_dynamic_thread, null, &refresh_dynamic_daemon, null) == 0 ) {
            module_debug(MODULE_VEHICLE_DIAGNOSTIC "Checker thread created");
        } else {
            module_debug(MODULE_VEHICLE_DIAGNOSTIC "Fatal erro during thead creation");
        }
    }
}

#define VH_GTK_LABEL_SET_TEXT_SYM(sym,label) \
    static gboolean sym##_gsource(gpointer data) { \
        char *value = data; \
        gtk_label_set_text(label, value); \
        free(data); \
        return false; \
    }

#define VH_REFRESH_LABEL(sym) \
    g_idle_add(sym##_gsource,sym(iface, show_freeze_frame_get_state()))

VH_GTK_LABEL_SET_TEXT_SYM(saej1979_data_engine_type_as_string, gui->engine.type)
VH_GTK_LABEL_SET_TEXT_SYM(saej1979_data_obd_standard_as_string, gui->engine.ecu.obdStandard)
VH_GTK_LABEL_SET_TEXT_SYM(saej1979_data_fuel_type_as_string, gui->engine.fuel.type)


static void refresh_one_time_internal() {
    final VehicleIFace* iface = config.ephemere.iface;
    if ( ! vehicle_explorer_error_feedback_obd(iface) ) {
        VH_REFRESH_LABEL(saej1979_data_engine_type_as_string);
        VH_REFRESH_LABEL(saej1979_data_obd_standard_as_string);
        VH_REFRESH_LABEL(saej1979_data_fuel_type_as_string);
    }
}

static void* refresh_one_time_daemon(void *arg) {
    refresh_dynamic_internal(); 
    refresh_one_time_internal(); 
    return null;
}

static void refresh_one_time() {
    pthread_t t;
    pthread_create(&t, null, &refresh_one_time_daemon, null);
}

static void* refresh_one_time_with_spinner_daemon(final void *arg) {
    final pthread_t * t = (pthread_t*)arg;
    gtk_spinner_start(gui->refreshIcon);
    pthread_join(*t, NULL);
    gtk_spinner_stop(gui->refreshIcon);
    free(t);
    return null;
}
static void refresh_one_time_with_spinner() {
    pthread_t *t,t1;
    t = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(t, null, &refresh_one_time_daemon, null);
    pthread_create(&t1, null, &refresh_one_time_with_spinner_daemon, t);
}
static gboolean graphs_on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    pthread_mutex_lock(&graphs_mutex);
    VehicleIFace* iface = config.ephemere.iface;
    char * graph_title = (char*)user_data;
    final Graph * graph = list_Graph_get_by_title(graphs, graph_title);
    assert(graph != null);

    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;

    const int margin_left = 50;
    const int margin_bottom = 30;
    const int margin_top = 30;

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14.0);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_text_extents_t ext;
    cairo_text_extents(cr, graph->title, &ext);
    cairo_move_to(cr, (width - ext.width) / 2 - ext.x_bearing, margin_top - 10);
    cairo_show_text(cr, graph->title);

    if (graph->data->size == 0) {
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 14.0);
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        const char *msg = "No data";
        cairo_text_extents_t ext;
        cairo_text_extents(cr, msg, &ext);
        cairo_move_to(cr, (width - ext.width) / 2 - ext.x_bearing,
                        (height + ext.height) / 2 - ext.y_bearing);
        cairo_show_text(cr, msg);
        pthread_mutex_unlock(&graphs_mutex);
        return FALSE;
    }

    GraphData * data_0 = graph->data->list[0];
    double min_val = data_0->data, max_val = data_0->data;
    double min_time = data_0->time, max_time = data_0->time;
    for (int i = 1; i < graph->data->size; i++) {
        GraphData * data = graph->data->list[i];
        if (data->data < min_val) min_val = data->data;
        if (data->data > max_val) max_val = data->data;
        if (data->time < min_time) min_time = data->time;
        if (data->time > max_time) max_time = data->time;
    }

    gboolean y_centered = FALSE;
    if (min_val == max_val) {
        y_centered = TRUE;
        min_val -= 0.5;
        max_val += 0.5;
    }

    double y_scale = (height - margin_top - margin_bottom) / (max_val - min_val);
    double x_scale = (width - margin_left - 10) / (max_time - min_time);
    double x_offset = margin_left;
    if (min_time == max_time) {
        x_scale = 0;
        x_offset = (width + margin_left) / 2.0;
    }

    int tick_count = 10;
    double y_tick_step = (max_val - min_val) / tick_count;
    double x_tick_step = (max_time - min_time) / tick_count;

    // Background grid lines for both axes
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_set_line_width(cr, 0.5);

    // Y grid lines
    for (int i = 0; i <= tick_count; i++) {
        double val = min_val + i * y_tick_step;
        double y = height - margin_bottom - (val - min_val) * y_scale;
        cairo_move_to(cr, margin_left, y);
        cairo_line_to(cr, width, y);
        cairo_stroke(cr);
    }

    // X grid lines
    for (int i = 0; i <= tick_count; i++) {
        double t = min_time + i * x_tick_step;
        double x = x_offset + (t - min_time) * x_scale;
        cairo_move_to(cr, x, margin_top);
        cairo_line_to(cr, x, height - margin_bottom);
        cairo_stroke(cr);
    }

    // Y-axis line
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, margin_left, margin_top);
    cairo_line_to(cr, margin_left, height - margin_bottom);
    cairo_stroke(cr);

    // Y-axis ticks and labels (short on axis only)
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 9.0);
    cairo_set_line_width(cr, 0.8);
    for (int i = 0; i <= tick_count; i++) {
        double val = min_val + i * y_tick_step;
        double y = height - margin_bottom - (val - min_val) * y_scale;
        cairo_move_to(cr, margin_left - 5, y);
        cairo_line_to(cr, margin_left, y);
        cairo_stroke(cr);
        char label[16];
        snprintf(label, sizeof(label), "%.1f", val);
        cairo_move_to(cr, margin_left - 45, y + 3);
        cairo_show_text(cr, label);
    }

    // Draw X-axis line
    double base_y = height - margin_bottom;
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, margin_left, base_y);
    cairo_line_to(cr, width, base_y);
    cairo_stroke(cr);

    // Draw X-axis ticks and labels
    cairo_set_font_size(cr, 9.0);
    cairo_set_line_width(cr, 0.8);
    for (int i = 0; i <= tick_count; i++) {
        double t = min_time + i * x_tick_step;
        double x = x_offset + (t - min_time) * x_scale;
        cairo_move_to(cr, x, base_y);
        cairo_line_to(cr, x, base_y + 5);
        cairo_stroke(cr);
        char label[16];
        snprintf(label, sizeof(label), "%.0f", t);
        cairo_move_to(cr, x - 10, base_y + 15);
        cairo_show_text(cr, label);
    }

    // Axis labels
    cairo_move_to(cr, 5, margin_top - 5);
    cairo_show_text(cr, graph->unit);
    cairo_move_to(cr, width - 60, height - 5);
    cairo_show_text(cr, "time (ms)");

    // Curve
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1.2);
    double x = x_offset + (data_0->time - min_time) * x_scale;
    double y = y_centered ? margin_top + (height - margin_top - margin_bottom) / 2.0
                          : height - margin_bottom - (data_0->data - min_val) * y_scale;
    cairo_move_to(cr, x, y);
    for (int i = 1; i < graph->data->size; i++) {
        final GraphData * data = graph->data->list[i];
        x = x_offset + (data->time - min_time) * x_scale;
        y = y_centered ? margin_top + (height - margin_top - margin_bottom) / 2.0
                       : height - margin_bottom - (data->data - min_val) * y_scale;
        cairo_line_to(cr, x, y);
    }
    cairo_stroke(cr);
    pthread_mutex_unlock(&graphs_mutex);
    return FALSE;
}

#define VH_GRAPHS_IS_ACTIVE_SET(title, unit) \
    ( strcmp(activeGraph, title) == 0 ) { \
        list_Graph_append(graphs, graph_new(drawing_area, activeGraph, unit)); \
    }

static void* graphs_add_daemon(void *arg) {
    pthread_mutex_lock(&graphs_mutex);
    static int graph_count = 0;
    VehicleIFace* iface = config.ephemere.iface;

    int active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(gui->graphs.list));
    if ( 0 <= active_index ) {
        GtkWidget *drawing_area = gtk_drawing_area_new();
        gtk_widget_set_size_request(drawing_area, 300, 300);
        final char * activeGraph = gtk_combo_box_text_get_active_text(gui->graphs.list);
        if VH_GRAPHS_IS_ACTIVE_SET("Speed", "km/h") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Coolant Temperature", "°C") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Intake Air Temperature", "°C") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Intake Air Manifold Pressure", "kPa") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Intake Air MAF Rate", "g/s") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Engine Speed", "r/min") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Fuel Pressure", "kPa") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Fuel Level", "%") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Fuel ethanol", "%") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Fuel Rail Pressure", "kPa") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Fuel rate", "L/h") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Fuel trim long term bank1", "%") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Fuel trim long term bank2", "%") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Fuel trim short term bank1", "%") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Fuel trim short term bank2", "%") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Injection timing", "°") 
        else if VH_GRAPHS_IS_ACTIVE_SET("Injection timing advance before TDC", "°") 
        else {
            bool sensor_found = false;
            for(int sensor_i = 1; sensor_i <= 8; sensor_i++) { 
                char graphTitle[64]; 
                snprintf(graphTitle, sizeof(graphTitle), "Oxygen sensor %d voltage", sensor_i); 
                if ( strcmp(activeGraph, graphTitle) == 0 ) { 
                    list_Graph_append(graphs, graph_new(drawing_area, activeGraph, "V")); 
                    sensor_found = true;
                    break;
                }
                snprintf(graphTitle, sizeof(graphTitle), "Oxygen sensor %d current", sensor_i); 
                if ( strcmp(activeGraph, graphTitle) == 0 ) { 
                    list_Graph_append(graphs, graph_new(drawing_area, activeGraph, "mA")); 
                    sensor_found = true;
                    break;
                }
                snprintf(graphTitle, sizeof(graphTitle), "Oxygen sensor %d air fuel equivalence ratio", sensor_i); 
                if ( strcmp(activeGraph, graphTitle) == 0 ) { 
                    list_Graph_append(graphs, graph_new(drawing_area, activeGraph, "ratio")); 
                    sensor_found = true;
                    break;
                }
            } 
            if ( ! sensor_found ) {
                log_msg(LOG_ERROR, "Unsupported type of graph '%'", activeGraph);
                pthread_mutex_unlock(&graphs_mutex);
                return null;
            }
        }
        g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(graphs_on_draw), strdup(activeGraph));
        gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(gui->graphs.list), active_index);

        final int col = graph_count % 2;
        final int row = graph_count / 2;

        gtk_grid_attach(GTK_GRID(gui->graphs.container), drawing_area, col, row, 1, 1);
        gtk_widget_show_all(GTK_WIDGET(gui->graphs.container));

        graph_count++;
    } else {
        error_feedback(gui->genericErrorFeedback, "Input error", "Please select the type of metric to display");
    }
    pthread_mutex_unlock(&graphs_mutex);
    return null;
}
static void graphs_add() {
    pthread_t t;
    pthread_create(&t, null, &graphs_add_daemon, null);
}
static void* graphs_reset_data_daemon(void *arg) {
    pthread_mutex_lock(&graphs_mutex);
    for(int i = 0; i < graphs->size; i++) {
        list_GraphData_free(graphs->list[i]->data);
        graphs->list[i]->data = list_GraphData_new();
    }
    graph_time_start_ms = 0;
    pthread_mutex_unlock(&graphs_mutex);
    return null;
}
static void graphs_reset_data() {
    pthread_t t;
    pthread_create(&t, null, &graphs_reset_data_daemon, null);
}
static void refresh_changed (GtkCheckMenuItem *checkmenuitem, gpointer user_data) {
    bool state = gtk_check_menu_item_get_active(checkmenuitem);
    config.vehicleExplorer.autoRefresh = state;
    config_store();
    if ( state ) {
        refresh_dynamic();        
    } else {
        THREAD_CANCEL(refresh_dynamic_thread);    
    }
}
static void show_window() {
    gtk_widget_show_now (gui->window);
    gtk_check_menu_item_set_active(gui->menuBar.autoRefresh,config.vehicleExplorer.autoRefresh);
    refresh_one_time();
    refresh_changed(gui->menuBar.autoRefresh,null);
}

static gboolean expander_show_child(gpointer data) {
    GtkWidget *widget = (GtkWidget *)data;
    if (!gtk_widget_get_realized(widget))
        gtk_widget_realize(widget);
    gtk_widget_set_visible(widget, true);
    gtk_widget_queue_draw(widget);
    return false;
}
static void expander_show_childs(GtkWidget *widget, gpointer data) {
    final const char * name = gtk_widget_get_name(widget);
    if ( strcmp(name,"GtkBox") == 0 || strcmp(name,"GtkGrid") == 0 ) {
        gtk_container_foreach((GtkContainer*)widget,
            expander_show_childs,
            null);
    } else {
        g_idle_add(expander_show_child, widget);
    }
}
static void expander_activate(GtkExpander* expander, gpointer data) {
    final bool state = gtk_expander_get_expanded(expander);
    if ( ! state ) {
        gtk_container_foreach((GtkContainer*)expander,
            expander_show_childs,
            null);
    }
}

static void expanders_default_state(GtkWidget *widget, gpointer data) {
    final const char * name = gtk_widget_get_name(widget);
    if ( strcmp(name,"GtkExpander") == 0 ) {
        g_signal_connect(G_OBJECT(widget),"activate",G_CALLBACK(expander_activate),NULL);
        gtk_container_foreach(
            GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(widget))),
            expanders_default_state, null
            );
    } else if ( strcmp(name,"GtkBox") == 0 || strcmp(name, "GtkGrid") == 0 ) {
        gtk_container_foreach(GTK_CONTAINER(widget),
            expanders_default_state, null);
    } else {
        gtk_widget_set_visible(widget, false);        
    }
}
#define VH_RETRIEVE_OX_SENSOR(i) \
    .sensor_##i = { \
        .voltage = GTK_LABEL(gtk_builder_get_object (builder, "vehicle-explorer-engine-sensors-"#i"-voltage")), \
        .current = GTK_LABEL(gtk_builder_get_object (builder, "vehicle-explorer-engine-sensors-"#i"-current")), \
        .ratio = GTK_LABEL(gtk_builder_get_object (builder, "vehicle-explorer-engine-sensors-"#i"-ratio")), \
    }
static void generic_error_feedback_ok() {
    gtk_widget_hide_on_main_thread(GTK_WIDGET(gui->genericErrorFeedback));
}
static void on_filter_check_toggled(GtkCheckMenuItem *check_item, gpointer user_data) {
    ECU *ecu = g_object_get_data(G_OBJECT(check_item), "ecu");
    if (gtk_check_menu_item_get_active(check_item)) {
        viface_recv_filter_add(config.ephemere.iface, ecu->address);
    } else {
        viface_recv_filter_rm(config.ephemere.iface, ecu->address);
    }
}
static GtkWidget *find_widget_for_ecu(GtkWidget *menu, ECU *ecu) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(menu));
    for (GList *l = children; l != NULL; l = l->next) {
        GtkWidget *w = GTK_WIDGET(l->data);
        ECU *attached = g_object_get_data(G_OBJECT(w), "ecu");
        if (attached == ecu) {
            g_list_free(children);
            return w;
        }
    }
    g_list_free(children);
    return null;
}
static void menubar_data_source_filter_by_filter_change(final char * type, Buffer * address) {
    if ( strcmp(type, "clear") == 0 ) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(gui->menuBar.data.source.filter_by_menu));
        for (GList *l = children; l != NULL; l = l->next) {
            GtkWidget *w = GTK_WIDGET(l->data);
            gtk_check_menu_item_set_active(w, false);
        }
        g_list_free(children);
    } else {
        final ECU * ecu = vehicle_search_ecu_by_address(config.ephemere.iface->vehicle, address);
        if ( ecu == null ) {
            log_msg(LOG_WARNING, "ecu with address '%s' not found", buffer_to_hex_string(address));
        } else {
            GtkWidget * w = find_widget_for_ecu(gui->menuBar.data.source.filter_by_menu, ecu);
            if ( w == null ) {
                log_msg(LOG_WARNING, "widget associated with: not found");
                vehicle_ecu_debug(ecu);
            } else {
                gtk_check_menu_item_set_active(w, strcmp(type,"add") == 0);
            }
        }
    }
}
static void menubar_data_source_filter_by_register(final ECU * ecu) {
    char *displayLabel;
    asprintf(&displayLabel, "%s (%s)", ecu->name, buffer_to_hex_string(ecu->address));
    GtkWidget *filter_check = find_widget_for_ecu(gui->menuBar.data.source.filter_by_menu, ecu);
    if ( filter_check == null ) {
        filter_check = gtk_check_menu_item_new_with_label(displayLabel);
        gtk_menu_shell_append(GTK_MENU_SHELL(gui->menuBar.data.source.filter_by_menu), filter_check);
        gtk_widget_show(filter_check);
        g_object_set_data(G_OBJECT(filter_check), "ecu", ecu);
        g_signal_connect(filter_check, "toggled", G_CALLBACK(on_filter_check_toggled), gui->menuBar.data.source.all);
    } else {
        gtk_menu_item_set_label(GTK_MENU_ITEM(filter_check), displayLabel);
    }
    free(displayLabel);
}
static void menubar_data_source_all(GtkMenuItem *item, gpointer user_data) {
    viface_recv_filter_clear(config.ephemere.iface);
}
void module_init_vehicle_explorer(final GtkBuilder *builder) {
    if ( gui == null ) {
        graphs = list_Graph_new();
        vehicleExplorerGui g = {
            .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-vehicle-explorer")),
            .refreshIcon = (GtkSpinner*)gtk_builder_get_object (builder, "window-vehicle-explorer-global-refresh"),
            .errorFeedback = ERROR_FEEDBACK_RETRIEVE_WINDOWS(builder),
            .menuBar = {
                .autoRefresh = (GtkCheckMenuItem*)gtk_builder_get_object(builder,"vehicle-explorer-menubar-autorefresh"),
                .freeze_frame_thread = null,
                .freeze_frame_error_popup = GTK_WINDOW(gtk_builder_get_object(builder,"vehicle-explorer-freeze-frame-error")),
                .showFreezeFrame = GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder,"vehicle-explorer-menubar-show-freezeframe")),
                .data = {
                    .source = {
                        .all = GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder, "vehicle-explorer-menubar-data-source-all")),
                        .filter_by = GTK_WIDGET(gtk_builder_get_object(builder, "vehicle-explorer-menubar-data-source-filter-by"))
                    }    
                }
            },
            .engine = {
                .expandableSection = (GtkBox *)gtk_builder_get_object(builder,"vehicle-explorer-expandable-section"),
                .load = GTK_PROGRESS_BAR(gtk_builder_get_object(builder,"vehicle-explorer-engine-load")),
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
            },
            .graphs = {
                .add = GTK_BUTTON(gtk_builder_get_object(builder,"vehicle-explorer-graphs-add")),
                .list = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder,"vehicle-explorer-graphs-list")),
                .container = GTK_GRID(gtk_builder_get_object(builder,"vehicle-explorer-graphs-container")),
                .resetData = GTK_BUTTON(gtk_builder_get_object(builder,"vehicle-explorer-graphs-reset-data"))
            },
            .genericErrorFeedback = GTK_MESSAGE_DIALOG(gtk_builder_get_object (builder, "vehicle-explorer-feedback-info"))
        };
        gtk_combo_box_text_append(g.graphs.list, NULL, "Speed");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Coolant Temperature");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Intake Air Temperature");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Intake Air Manifold Pressure");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Intake Air MAF Rate");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Engine Speed");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Fuel Pressure");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Fuel Level");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Fuel ethanol");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Fuel Rail Pressure");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Fuel rate");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Fuel trim long term bank1");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Fuel trim long term bank2");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Fuel trim short term bank1");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Fuel trim short term bank2");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Injection timing");
        gtk_combo_box_text_append(g.graphs.list, NULL, "Injection timing advance before TDC");
        for(int sensor_i = 1; sensor_i <= 8; sensor_i++) { 
            char graphTitle[64]; 
            snprintf(graphTitle, sizeof(graphTitle), "Oxygen sensor %d voltage", sensor_i); 
            gtk_combo_box_text_append(g.graphs.list, NULL, graphTitle);
            snprintf(graphTitle, sizeof(graphTitle), "Oxygen sensor %d current", sensor_i); 
            gtk_combo_box_text_append(g.graphs.list, NULL, graphTitle);
            snprintf(graphTitle, sizeof(graphTitle), "Oxygen sensor %d air fuel equivalence ratio", sensor_i); 
            gtk_combo_box_text_append(g.graphs.list, NULL, graphTitle);
        } 

        gui = (vehicleExplorerGui*)malloc(sizeof(vehicleExplorerGui));
        (*gui) = g;
        counter_init_with(g.engine.vehicleSpeed,"counter_85_2_255_0_0_255.png");
        counter_init_with(g.engine.speed,"counter_85_2_255_0_0_255.png");
        counter_init_with(g.engine.fuel.pressure,"gaugehalf_225_5_255_0_0_255.png");
        counter_init_with(g.engine.fuel.rail.pressure,"gaugehalf_225_5_255_0_0_255.png");        
        expanders_default_state((GtkWidget*)gui->engine.expandableSection,null);
        g_signal_connect(G_OBJECT(gui->window),"delete-event",G_CALLBACK(onclose),NULL);
        g_signal_connect(G_OBJECT(g.menuBar.freeze_frame_error_popup),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        error_feedback_windows_init(gui->errorFeedback);
        g_signal_connect(G_OBJECT(gui->genericErrorFeedback),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        gtk_builder_add_callback_symbol(builder,"vehicle-explorer-generic-error-feedback-ok",&generic_error_feedback_ok);
        gtk_builder_add_callback_symbol(builder, "vehicle-explorer-graphs-reset-data", &graphs_reset_data);
        gtk_builder_add_callback_symbol(builder,"vehicle-explorer-graphs-add",&graphs_add);
        gtk_builder_add_callback_symbol(builder,"show-window-vehicle-explorer",&show_window);
        gtk_builder_add_callback_symbol(builder,"window-vehicle-explorer-global-refresh-click",&refresh_one_time_with_spinner);
        gtk_builder_add_callback_symbol(builder,"window-vehicle-explorer-global-refresh-changed",G_CALLBACK(&refresh_changed));
        gtk_builder_add_callback_symbol(builder,"vehicle-explorer-freeze-frame-error-ok",G_CALLBACK(&freeze_frame_error_ok));
        gtk_builder_add_callback_symbol(builder,"window-vehicle-explorer-data-freeze-frame",&data_freeze_frame);
        g_signal_connect(gui->menuBar.data.source.all, "activate", G_CALLBACK(menubar_data_source_all), null);

        GtkWidget *filter_menu = gtk_menu_new();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(gui->menuBar.data.source.filter_by), GTK_WIDGET(filter_menu));
        gui->menuBar.data.source.filter_by_menu = filter_menu;
        ehh_register(config.ephemere.iface->vehicle->internal.events.onECURegister, menubar_data_source_filter_by_register);
        ehh_register(config.ephemere.iface->vehicle->internal.events.onFilterChange, menubar_data_source_filter_by_filter_change);

    } else {
        module_debug(MODULE_VEHICLE_DIAGNOSTIC "module already initialized");
    }
}


