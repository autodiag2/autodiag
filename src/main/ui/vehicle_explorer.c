#include "ui/vehicle_explorer.h"

static vehicleExplorerGui *gui = null;
static pthread_t *refresh_dynamic_thread = null;
static ad_list_Graph *graphs = null;
static pthread_mutex_t graphs_mutex;
static bool graphs_should_refresh = false;

MENUBAR_DATA_ALL_IN_ONE

static MetricType metric_from_title(const char *activeGraph, int *out_arg) {
    *out_arg = 0;
    if (strcmp(activeGraph, "Speed") == 0) return METRIC_SPEED;
    if (strcmp(activeGraph, "Coolant Temperature") == 0) return METRIC_COOLANT_TEMP;
    if (strcmp(activeGraph, "Intake Air Temperature") == 0) return METRIC_INTAKE_AIR_TEMP;
    if (strcmp(activeGraph, "Intake Air Manifold Pressure") == 0) return METRIC_INTAKE_MANIFOLD_PRESSURE;
    if (strcmp(activeGraph, "Intake Air MAF Rate") == 0) return METRIC_MAF_RATE;
    if (strcmp(activeGraph, "Engine Speed") == 0) return METRIC_ENGINE_SPEED;
    if (strcmp(activeGraph, "Fuel Pressure") == 0) return METRIC_FUEL_PRESSURE;
    if (strcmp(activeGraph, "Fuel Level") == 0) return METRIC_FUEL_LEVEL;
    if (strcmp(activeGraph, "Fuel ethanol") == 0) return METRIC_FUEL_ETHANOL;
    if (strcmp(activeGraph, "Fuel Rail Pressure") == 0) return METRIC_FUEL_RAIL_PRESSURE;
    if (strcmp(activeGraph, "Fuel rate") == 0) return METRIC_FUEL_RATE;
    if (strcmp(activeGraph, "Fuel trim long term bank1") == 0) return METRIC_FUEL_TRIM_LT_B1;
    if (strcmp(activeGraph, "Fuel trim long term bank2") == 0) return METRIC_FUEL_TRIM_LT_B2;
    if (strcmp(activeGraph, "Fuel trim short term bank1") == 0) return METRIC_FUEL_TRIM_ST_B1;
    if (strcmp(activeGraph, "Fuel trim short term bank2") == 0) return METRIC_FUEL_TRIM_ST_B2;
    if (strcmp(activeGraph, "Injection timing") == 0) return METRIC_INJECTION_TIMING;
    if (strcmp(activeGraph, "Injection timing advance before TDC") == 0) return METRIC_INJECTION_ADV_BTDC;

    for (int sensor_i = 1; sensor_i <= 8; sensor_i++) {
        char graphTitle[64];
        snprintf(graphTitle, sizeof(graphTitle), "Oxygen sensor %d voltage", sensor_i);
        if (strcmp(activeGraph, graphTitle) == 0) { *out_arg = sensor_i; return METRIC_OX_VOLTAGE; }
        snprintf(graphTitle, sizeof(graphTitle), "Oxygen sensor %d current", sensor_i);
        if (strcmp(activeGraph, graphTitle) == 0) { *out_arg = sensor_i; return METRIC_OX_CURRENT; }
        snprintf(graphTitle, sizeof(graphTitle), "Oxygen sensor %d air fuel equivalence ratio", sensor_i);
        if (strcmp(activeGraph, graphTitle) == 0) { *out_arg = sensor_i; return METRIC_OX_RATIO; }
    }
    return METRIC_SPEED;
}

static double metric_get(VehicleIFace *iface, int dataFrameNumber, MetricType t, int arg, double error) {
    switch (t) {
        case METRIC_SPEED: return saej1979_data_vehicle_speed(iface, dataFrameNumber);
        case METRIC_COOLANT_TEMP: return saej1979_data_engine_coolant_temperature(iface, dataFrameNumber);
        case METRIC_INTAKE_AIR_TEMP: return saej1979_data_intake_air_temperature(iface, dataFrameNumber);
        case METRIC_INTAKE_MANIFOLD_PRESSURE: return saej1979_data_intake_manifold_pressure(iface, dataFrameNumber);
        case METRIC_MAF_RATE: return saej1979_data_maf_air_flow_rate(iface, dataFrameNumber);
        case METRIC_ENGINE_SPEED: return saej1979_data_engine_speed(iface, dataFrameNumber);
        case METRIC_FUEL_PRESSURE: return saej1979_data_fuel_pressure(iface, dataFrameNumber);
        case METRIC_FUEL_LEVEL: return saej1979_data_fuel_tank_level_input(iface, dataFrameNumber);
        case METRIC_FUEL_ETHANOL: return saej1979_data_ethanol_fuel_percent(iface, dataFrameNumber);
        case METRIC_FUEL_RAIL_PRESSURE: return saej1979_data_frp_relative(iface, dataFrameNumber);
        case METRIC_FUEL_RATE: return saej1979_data_engine_fuel_rate(iface, dataFrameNumber);
        case METRIC_FUEL_TRIM_LT_B1: return saej1979_data_long_term_fuel_trim_bank_1(iface, dataFrameNumber);
        case METRIC_FUEL_TRIM_LT_B2: return saej1979_data_long_term_fuel_trim_bank_2(iface, dataFrameNumber);
        case METRIC_FUEL_TRIM_ST_B1: return saej1979_data_short_term_fuel_trim_bank_1(iface, dataFrameNumber);
        case METRIC_FUEL_TRIM_ST_B2: return saej1979_data_short_term_fuel_trim_bank_2(iface, dataFrameNumber);
        case METRIC_INJECTION_TIMING: return saej1979_data_fuel_injection_timing(iface, dataFrameNumber);
        case METRIC_INJECTION_ADV_BTDC: return saej1979_data_timing_advance_cycle_1(iface, dataFrameNumber);
        case METRIC_OX_VOLTAGE: return saej1979_data_oxygen_sensor_voltage(iface, dataFrameNumber, arg);
        case METRIC_OX_CURRENT: return saej1979_data_oxygen_sensor_current(iface, dataFrameNumber, arg);
        case METRIC_OX_RATIO: return saej1979_data_oxygen_sensor_air_fuel_equiv_ratio(iface, dataFrameNumber, arg);
    }
    return error;
}

static double metric_error(MetricType t) {
    switch (t) {
        case METRIC_INTAKE_AIR_TEMP: return SAEJ1979_DATA_ENGINE_INTAKE_AIR_TEMPERATURE_ERROR;
        case METRIC_INTAKE_MANIFOLD_PRESSURE: return SAEJ1979_DATA_INTAKE_MANIFOLD_PRESSURE_ERROR;
        case METRIC_MAF_RATE: return SAEJ1979_DATA_VEHICLE_MAF_AIR_FLOW_RATE_ERROR;
        case METRIC_FUEL_LEVEL: return SAEJ1979_DATA_FUEL_TANK_LEVEL_INPUT_ERROR;
        case METRIC_FUEL_ETHANOL: return SAEJ1979_DATA_ETHANOL_FUEL_PERCENT_ERROR;
        case METRIC_FUEL_RAIL_PRESSURE: return SAEJ1979_DATA_FRP_RELATIVE_ERROR;
        case METRIC_FUEL_RATE: return SAEJ1979_DATA_ENGINE_FUEL_RATE_ERROR;
        case METRIC_FUEL_TRIM_LT_B1: return SAEJ1979_DATA_FUEL_TRIM_ERROR;
        case METRIC_FUEL_TRIM_LT_B2: return SAEJ1979_DATA_FUEL_TRIM_ERROR;
        case METRIC_FUEL_TRIM_ST_B1: return SAEJ1979_DATA_FUEL_TRIM_ERROR;
        case METRIC_FUEL_TRIM_ST_B2: return SAEJ1979_DATA_FUEL_TRIM_ERROR;
        case METRIC_INJECTION_TIMING: return SAEJ1979_DATA_FUEL_INJECTION_TIMING_ERROR;
        case METRIC_INJECTION_ADV_BTDC: return SAEJ1979_DATA_TIMING_ADVANCE_CYCLE_1_ERROR;
        case METRIC_OX_VOLTAGE: return SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_ERROR;
        case METRIC_OX_CURRENT: return SAEJ1979_DATA_OXYGEN_SENSOR_CURRENT_ERROR;
        case METRIC_OX_RATIO: return SAEJ1979_DATA_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_ERROR;
        default: {
            return NAN;
        }
    }
    return NAN;
}

static void button_click_clean_up_routine(void *arg) {
    thread_viface_cleanup_routine(arg);
    gtk_spinner_stop(gui->refreshIcon);
}

static bool get_data_frame_selected() {
    return gtk_check_menu_item_get_active(gui->menuBar.showFreezeFrame) ? 0 : AD_SAEJ1979_DATA_FRAME_LIVE;
}

static void freeze_frame_error_ok() {
    gtk_widget_hide_on_main_thread(GTK_WIDGET(gui->menuBar.freeze_frame_error_popup));
}

static bool error_feedback(GtkMessageDialog *widget, char *title, char *msg) {
    if (msg == null && title == null) return false;

    if (msg == null) msg = strdup(" ");
    if (title != null) gtk_window_set_title(GTK_WINDOW(widget), title);
    gtk_message_dialog_set_markup(widget, msg);
    gtk_widget_show_on_main_thread(GTK_WIDGET(widget));
    return true;
}

static void data_freeze_frame_internal() {
    final VehicleIFace *iface = config.ephemere.iface;
    if (vehicle_explorer_error_feedback_obd(iface)) return;

    gtk_spinner_start(gui->refreshIcon);
    if (!saej1979_data_freeze_frame(iface)) {
        gtk_widget_show_on_main_thread(GTK_WIDGET(gui->menuBar.freeze_frame_error_popup));
    }
}

THREAD_WRITE_DAEMON(
    data_freeze_frame_daemon,
    data_freeze_frame_internal,
    button_click_clean_up_routine, gui->menuBar.freeze_frame_thread
)

static void data_freeze_frame() {
    thread_allocate_and_start(&gui->menuBar.freeze_frame_thread, &data_freeze_frame_daemon);
}

#define VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM_WITH_SIGNAL(sym, signal_path, bar) \
    VH_GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(sym, \
        double, \
        ad_signal_get(signal_path)->rv_min,ad_signal_get(signal_path)->rv_max, NAN, \
        gprintf("%%.2f %s", ad_signal_get(signal_path)->unit), \
        bar \
    )

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
    if (*currentECUvoltage != SAEJ1979_DATA_ECU_VOLTAGE_ERROR) {
        gtk_widget_printf(GTK_WIDGET(gui->engine.ecu.voltage), "%.3f V", *currentECUvoltage);
    }
    free(data);
    return false;
}

static gboolean saej1979_data_seconds_since_engine_start_gsource(gpointer data) {
    int *secondsSinceStart = data;
    if (*secondsSinceStart != SAEJ1979_DATA_SECONDS_SINCE_ENGINE_START_ERROR) {
        gtk_widget_printf(GTK_WIDGET(gui->engine.secondsSinceStart), "%d secs", *secondsSinceStart);
    }
    free(data);
    return false;
}
typedef struct {
    double value;
    GtkProgressBar * bar;
    ad_object_vehicle_signal * signal;
} vh_sensor_refresh_params;

static gboolean vh_refresh_widget_v2_generic(gpointer arg) {
    vh_sensor_refresh_params * params = (vh_sensor_refresh_params*)arg;
    char * fmt = gprintf("%%.2f %s", params->signal->unit);
    gtk_progress_bar_fill_from_double(params->bar, params->value, params->signal->rv_min, params->signal->rv_max, NAN, fmt);
    free(fmt);
    free(params);
    return false;
}
#define VH_REFRESH_WIDGET_V2(w,signal_path) \
    if (VH_SHOULD_REFRESH_WIDGET(w)) { \
        ad_object_vehicle_signal * signal = ad_signal_get(signal_path); \
        if ( signal == null ) { \
            log_err("Cannot find signal %s", signal_path); \
        } else { \
            iface->lock(iface); \
            int frameNumber = get_data_frame_selected(); \
            double result_value = NAN; \
            if ( frameNumber == AD_SAEJ1979_DATA_FRAME_LIVE ) { \
                if ( ! viface_use_signal(iface, signal, &result_value, "01", null) ) { \
                    log_warn("error while retrieving sensor : %s", signal_path);    \
                } \
            } else { \
                char * frameNumberStr = gprintf("%02hhX", frameNumber); \
                if ( ! viface_use_signal(iface, signal, &result_value, "02", frameNumberStr, null) ) { \
                    log_warn("error while retrieving sensor : %s", signal_path);    \
                } \
            } \
            iface->unlock(iface); \
            vh_sensor_refresh_params * params = (vh_sensor_refresh_params*)malloc(sizeof(vh_sensor_refresh_params)); \
            params->value = result_value; \
            params->signal = signal; \
            params->bar = w; \
            g_idle_add(vh_refresh_widget_v2_generic, params); \
        } \
    }

#define VH_REFRESH_WIDGET(w,data_gen,type) \
    if (VH_SHOULD_REFRESH_WIDGET(w)) { \
        g_idle_add(data_gen##_gsource, type##dup(data_gen(iface, get_data_frame_selected()))); \
    }

#define VH_REFRESH_GRAPH_GSOURCE_SYM(name) static gboolean name(gpointer data)
VH_REFRESH_GRAPH_GSOURCE_SYM(graph_refresh_gsource) {
    final Graph *graph = (Graph*)data;
    gtk_widget_queue_draw(graph->widget);
    return false;
}

static gboolean saej1979_data_fuel_system_status_gsource(gpointer data) {
    char *status = data;
    if (status != null) {
        gtk_widget_printf(GTK_WIDGET(gui->engine.fuel.status), "%s", status);
    }
    free(data);
    return false;
}

#define VH_OX_SENSOR_GSOURCE_SYM(sensor_i) \
    static gboolean saej1979_data_oxygen_sensor_voltage_##sensor_i##_gsource(gpointer data) { \
        double *volts = data; \
        if (volts != null) { \
            gtk_widget_printf(GTK_WIDGET(gui->engine.oxSensors.sensor_##sensor_i.voltage), "%.2f V", *volts); \
        } \
        free(data); \
        return false; \
    } \
    static gboolean saej1979_data_oxygen_sensor_current_##sensor_i##_gsource(gpointer data) { \
        int *current = data; \
        if (current != null) { \
            gtk_widget_printf(GTK_WIDGET(gui->engine.oxSensors.sensor_##sensor_i.current), "%d mA", *current); \
        } \
        free(data); \
        return false; \
    } \
    static gboolean saej1979_data_oxygen_sensor_air_fuel_equiv_ratio_##sensor_i##_gsource(gpointer data) { \
        double *ratio = data; \
        if (ratio != null) { \
            gtk_widget_printf(GTK_WIDGET(gui->engine.oxSensors.sensor_##sensor_i.ratio), "%.2f ratio", *ratio); \
        } \
        free(data); \
        return false; \
    }

VH_OX_SENSOR_GSOURCE_SYM(1) VH_OX_SENSOR_GSOURCE_SYM(2) VH_OX_SENSOR_GSOURCE_SYM(3)
VH_OX_SENSOR_GSOURCE_SYM(4) VH_OX_SENSOR_GSOURCE_SYM(5) VH_OX_SENSOR_GSOURCE_SYM(6)
VH_OX_SENSOR_GSOURCE_SYM(7) VH_OX_SENSOR_GSOURCE_SYM(8)

#define VH_REFRESH_OX_SENSOR(sensor_i) \
    if (VH_SHOULD_REFRESH_WIDGET(gui->engine.oxSensors.sensor_##sensor_i.voltage)) { \
        g_idle_add(saej1979_data_oxygen_sensor_voltage_##sensor_i##_gsource, doubledup(saej1979_data_oxygen_sensor_voltage(iface, get_data_frame_selected(), sensor_i))); \
    } \
    if (VH_SHOULD_REFRESH_WIDGET(gui->engine.oxSensors.sensor_##sensor_i.current)) { \
        g_idle_add(saej1979_data_oxygen_sensor_current_##sensor_i##_gsource, intdup(saej1979_data_oxygen_sensor_current(iface, get_data_frame_selected(), sensor_i))); \
    } \
    if (VH_SHOULD_REFRESH_WIDGET(gui->engine.oxSensors.sensor_##sensor_i.ratio)) { \
        g_idle_add(saej1979_data_oxygen_sensor_air_fuel_equiv_ratio_##sensor_i##_gsource, doubledup(saej1979_data_oxygen_sensor_air_fuel_equiv_ratio(iface, get_data_frame_selected(), sensor_i))); \
    }

static gboolean saej1979_data_tests_gsource(gpointer data) {
    GList *glist = gtk_container_get_children(gui->engine.tests);
    while (glist != null) {
        gtk_container_remove(gui->engine.tests, glist->data);
        glist = glist->next;
    }
    ad_list_SAEJ1979_DATA_Test *testsList = data;
    for (unsigned i = 0; i < testsList->size; i++) {
        SAEJ1979_DATA_Test *test = testsList->list[i];
        GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5));
        gtk_container_add(GTK_CONTAINER(box), gtk_label_new(test->name));
        gtk_container_add(GTK_CONTAINER(box), gtk_label_new(test->completed ? "complete" : "incomplete"));
        gtk_container_add(gui->engine.tests, GTK_WIDGET(box));
        gtk_widget_show_all(GTK_WIDGET(box));
        free(test->name);
    }
    ad_list_SAEJ1979_DATA_Test_free(data);
    return false;
}

static void ad_list_Graph_append_data_for_series(Graph *g, unsigned series_idx, double value) {
    if (!g || !g->series) return;
    if (g->series->size <= series_idx) return;

    GraphSeries *s = g->series->list[series_idx];
    if (!s) return;

    GraphData *d = malloc(sizeof(*d));
    d->time = graph_time_ms_ellapsed(g);
    d->data = value;
    ad_list_GraphData_append(s->data, d);
}

static void graphs_refresh_all_series(VehicleIFace *iface, int dataFrameNumber) {
    if (!graphs) return;
    if (!graphs_should_refresh) return;

    for (unsigned gi = 0; gi < graphs->size; gi++) {
        Graph *g = graphs->list[gi];
        if (!g || !g->series) continue;
        if (!VH_SHOULD_REFRESH_WIDGET(g->widget)) continue;

        for (unsigned si = 0; si < g->series->size; si++) {
            GraphSeries *s = g->series->list[si];
            if (!s) continue;
            double err = metric_error(s->type);
            double v = metric_get(iface, dataFrameNumber, s->type, s->arg, err);
            if (v != err) ad_list_Graph_append_data_for_series(g, si, v);
        }

        g_idle_add(graph_refresh_gsource, g);
    }
}

static bool refresh_dynamic_internal() {
    final VehicleIFace *iface = config.ephemere.iface;
    if (vehicle_explorer_error_feedback_obd(iface)) return false;

    int dataFrameNumber = get_data_frame_selected();
    VH_REFRESH_WIDGET_V2(gui->engine.speed,                 "SAEJ1979.engine_speed");
    VH_REFRESH_WIDGET_V2(gui->engine.vehicleSpeed,          "SAEJ1979.vehicle_speed");
    VH_REFRESH_WIDGET_V2(gui->engine.load,                  "SAEJ1979.engine_load");
    VH_REFRESH_WIDGET_V2(gui->engine.coolant.temperature,   "SAEJ1979.coolant_temp");
    VH_REFRESH_WIDGET_V2(gui->engine.fuel.pressure,         "SAEJ1979.fuel_pressure");
    VH_REFRESH_WIDGET(gui->engine.intakeAir.temperature,                      saej1979_data_intake_air_temperature,       int);
    VH_REFRESH_WIDGET(gui->engine.intakeAir.manifoldPressure,                 saej1979_data_intake_manifold_pressure,     int);
    VH_REFRESH_WIDGET(gui->engine.intakeAir.mafRate,                          saej1979_data_maf_air_flow_rate,            double);
    VH_REFRESH_WIDGET(gui->engine.ecu.voltage,                                saej1979_data_ecu_voltage,                  double);
    VH_REFRESH_WIDGET(gui->engine.secondsSinceStart,                          saej1979_data_seconds_since_engine_start,   int);
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
    graphs_refresh_all_series(iface, dataFrameNumber);
    pthread_mutex_unlock(&graphs_mutex);

    if (VH_SHOULD_REFRESH_WIDGET(gtk_widget_get_parent(GTK_WIDGET(gui->engine.tests)))) {
        ad_list_SAEJ1979_DATA_Test *testsList = saej1979_data_tests(iface, dataFrameNumber, false);
        if (testsList != null) g_idle_add(saej1979_data_tests_gsource, testsList);
    }

    if (VH_SHOULD_REFRESH_WIDGET(gui->engine.fuel.status)) {
        char **status = saej1979_data_fuel_system_status(iface, dataFrameNumber);
        if (status != null) g_idle_add(saej1979_data_fuel_system_status_gsource, status[0]);
    }

    return true;
}

static void *refresh_dynamic_daemon(void *arg) {
    pthread_cleanup_push(thread_viface_cleanup_routine, null);
    const struct timespec req = {
        .tv_sec = config.vehicleExplorer.refreshRateS,
        .tv_nsec = (config.vehicleExplorer.refreshRateS - ((int)config.vehicleExplorer.refreshRateS)) * 1000000000
    };
    while (refresh_dynamic_internal()) {
        if (nanosleep(&req, null) == -1) break;
    }
    pthread_cleanup_pop(1);
    return null;
}

static void refresh_dynamic() {
    if (refresh_dynamic_thread == null) {
        refresh_dynamic_thread = (pthread_t*)malloc(sizeof(pthread_t));
        if (pthread_create(refresh_dynamic_thread, null, &refresh_dynamic_daemon, null) == 0) {
            log_msg(LOG_DEBUG, "Checker thread created");
        } else {
            log_msg(LOG_DEBUG, "Fatal error during thread creation");
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
    g_idle_add(sym##_gsource, sym(iface, get_data_frame_selected()))

VH_GTK_LABEL_SET_TEXT_SYM(saej1979_data_engine_type_as_string, gui->engine.type)
VH_GTK_LABEL_SET_TEXT_SYM(saej1979_data_obd_standard_as_string, gui->engine.ecu.obdStandard)
VH_GTK_LABEL_SET_TEXT_SYM(saej1979_data_fuel_type_as_string, gui->engine.fuel.type)

static void refresh_one_time_internal() {
    final VehicleIFace *iface = config.ephemere.iface;
    if (!vehicle_explorer_error_feedback_obd(iface)) {
        VH_REFRESH_LABEL(saej1979_data_engine_type_as_string);
        VH_REFRESH_LABEL(saej1979_data_obd_standard_as_string);
        VH_REFRESH_LABEL(saej1979_data_fuel_type_as_string);
    }
}

static void *refresh_one_time_daemon(void *arg) {
    refresh_dynamic_internal();
    refresh_one_time_internal();
    return null;
}

static void refresh_one_time() {
    pthread_t t;
    pthread_create(&t, null, &refresh_one_time_daemon, null);
}

static void *refresh_one_time_with_spinner_daemon(final void *arg) {
    final pthread_t *t = (pthread_t*)arg;
    gtk_spinner_start(gui->refreshIcon);
    pthread_join(*t, NULL);
    gtk_spinner_stop(gui->refreshIcon);
    free(t);
    return null;
}

static void refresh_one_time_with_spinner() {
    pthread_t *t, t1;
    t = (pthread_t*)malloc(sizeof(pthread_t));
    pthread_create(t, null, &refresh_one_time_daemon, null);
    pthread_create(&t1, null, &refresh_one_time_with_spinner_daemon, t);
}

static void curve_color(unsigned i, double *r, double *g, double *b) {
    if (i == 0) { *r = 1.0; *g = 0.0; *b = 0.0; return; }
    if (i == 1) { *r = 0.0; *g = 0.0; *b = 1.0; return; }
    static const double pal[][3] = {
        {0.0, 0.6, 0.0},
        {1.0, 0.5, 0.0},
        {0.6, 0.0, 0.6},
        {0.0, 0.6, 0.6},
        {0.3, 0.3, 0.3},
    };
    unsigned k = (i - 2) % (sizeof(pal)/sizeof(pal[0]));
    *r = pal[k][0]; *g = pal[k][1]; *b = pal[k][2];
}
static gboolean graphs_on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    pthread_mutex_lock(&graphs_mutex);

    char *graph_title = (char*)user_data;
    Graph *graph = ad_list_Graph_get_by_title(graphs, graph_title);
    assert(graph != null);

    GtkAllocation a;
    gtk_widget_get_allocation(widget, &a);
    const int width = a.width;
    const int height = a.height;

    const int margin_top = 30;
    const int margin_bottom = 30;
    const int axis_gap = 40;
    const int plot_right_pad = 10;

    const unsigned series_n = (graph->series ? graph->series->size : 0);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14.0);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_text_extents_t ext;
    cairo_text_extents(cr, graph->title ? graph->title : "", &ext);
    cairo_move_to(cr, (width - ext.width) / 2 - ext.x_bearing, margin_top - 10);
    cairo_show_text(cr, graph->title ? graph->title : "");

    bool has_any = false;
    double min_time = 0, max_time = 0;

    for (unsigned si = 0; si < series_n; si++) {
        GraphSeries *s = graph->series->list[si];
        if (!s || !s->data || s->data->size == 0) continue;
        GraphData *d0 = s->data->list[0];
        if (!has_any) {
            min_time = d0->time;
            max_time = d0->time;
            has_any = true;
        }
        for (unsigned i = 0; i < s->data->size; i++) {
            GraphData *d = s->data->list[i];
            if (d->time < min_time) min_time = d->time;
            if (max_time < d->time) max_time = d->time;
        }
    }

    if (!has_any) {
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 14.0);
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        const char *msg = "No data";
        cairo_text_extents_t ext2;
        cairo_text_extents(cr, msg, &ext2);
        cairo_move_to(cr, (width - ext2.width) / 2 - ext2.x_bearing,
                         (height + ext2.height) / 2 - ext2.y_bearing);
        cairo_show_text(cr, msg);
        pthread_mutex_unlock(&graphs_mutex);
        return FALSE;
    }

    const int left_extra = (series_n > 0 ? (int)(series_n - 1) * axis_gap : 0);
    const int plot_left = 50 + left_extra;
    const int plot_right = width - plot_right_pad;
    const int plot_top = margin_top;
    const int plot_bottom = height - margin_bottom;

    const double plot_h = (double)(plot_bottom - plot_top);
    const double plot_w = (double)(plot_right - plot_left);

    double x_scale = 0.0;
    if (min_time < max_time) x_scale = plot_w / (max_time - min_time);

    const int tick_count = 8;

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, plot_left, plot_top);
    cairo_line_to(cr, plot_left, plot_bottom);
    cairo_stroke(cr);

    cairo_move_to(cr, plot_left, plot_bottom);
    cairo_line_to(cr, plot_right, plot_bottom);
    cairo_stroke(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 9.0);

    if (min_time < max_time && 1 < tick_count) {
        double x_tick_step = (max_time - min_time) / (double)tick_count;

        cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
        cairo_set_line_width(cr, 0.5);
        for (int ti = 0; ti <= tick_count; ti++) {
            double t = min_time + (double)ti * x_tick_step;
            double x = (double)plot_left + (t - min_time) * x_scale;
            cairo_move_to(cr, x, plot_top);
            cairo_line_to(cr, x, plot_bottom);
            cairo_stroke(cr);
        }

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 0.8);
        for (int ti = 0; ti <= tick_count; ti++) {
            double t = min_time + (double)ti * x_tick_step;
            double x = (double)plot_left + (t - min_time) * x_scale;
            cairo_move_to(cr, x, plot_bottom);
            cairo_line_to(cr, x, plot_bottom + 5);
            cairo_stroke(cr);

            char label[32];
            snprintf(label, sizeof(label), "%.0f", t);
            cairo_text_extents_t tx;
            cairo_text_extents(cr, label, &tx);
            cairo_move_to(cr, x - tx.width / 2 - tx.x_bearing, plot_bottom + 15);
            cairo_show_text(cr, label);
        }
    }

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, plot_right - 60, height - 5);
    cairo_show_text(cr, "time (ms)");

    for (unsigned si = 0; si < series_n; si++) {
        GraphSeries *s = graph->series->list[si];
        if (!s || !s->data || s->data->size == 0) continue;

        double rr, gg, bb;
        curve_color(si, &rr, &gg, &bb);

        double s_min = s->data->list[0]->data;
        double s_max = s->data->list[0]->data;
        for (unsigned i = 1; i < s->data->size; i++) {
            double v = s->data->list[i]->data;
            if (v < s_min) s_min = v;
            if (s_max < v) s_max = v;
        }
        if (s_min == s_max) {
            s_min -= 0.5;
            s_max += 0.5;
        }

        const double y_scale = plot_h / (s_max - s_min);
        const int axis_x = plot_left - (int)si * axis_gap;

        cairo_set_source_rgb(cr, rr, gg, bb);
        cairo_set_line_width(cr, 1.0);
        cairo_move_to(cr, axis_x, plot_top);
        cairo_line_to(cr, axis_x, plot_bottom);
        cairo_stroke(cr);

        if (1 < tick_count) {
            double y_tick_step = (s_max - s_min) / (double)tick_count;

            cairo_set_source_rgb(cr, rr, gg, bb);
            cairo_set_line_width(cr, 0.8);

            for (int ti = 0; ti <= tick_count; ti++) {
                double val = s_min + (double)ti * y_tick_step;
                double y = (double)plot_bottom - (val - s_min) * y_scale;

                cairo_move_to(cr, axis_x - 5, y);
                cairo_line_to(cr, axis_x, y);
                cairo_stroke(cr);

                char label[32];
                snprintf(label, sizeof(label), "%.2f", val);

                cairo_text_extents_t tx;
                cairo_text_extents(cr, label, &tx);
                cairo_move_to(cr, axis_x - 8 - tx.width - tx.x_bearing, y + 3);
                cairo_show_text(cr, label);
            }
        }

        if (s->unit) {
            cairo_text_extents_t ux;
            cairo_text_extents(cr, s->unit, &ux);
            cairo_move_to(cr, axis_x - ux.width / 2 - ux.x_bearing, plot_top - 6);
            cairo_show_text(cr, s->unit);
        }

        cairo_set_source_rgb(cr, rr, gg, bb);
        cairo_set_line_width(cr, (si == 0) ? 1.6 : 1.2);

        GraphData *d0 = s->data->list[0];
        double x0 = (double)plot_left;
        if (min_time < max_time) x0 += (d0->time - min_time) * x_scale;

        double y0 = (double)plot_bottom - (d0->data - s_min) * y_scale;
        cairo_move_to(cr, x0, y0);

        for (unsigned i = 1; i < s->data->size; i++) {
            GraphData *d = s->data->list[i];
            double x = (double)plot_left;
            if (min_time < max_time) x += (d->time - min_time) * x_scale;
            double y = (double)plot_bottom - (d->data - s_min) * y_scale;
            cairo_line_to(cr, x, y);
        }
        cairo_stroke(cr);
    }

    cairo_set_font_size(cr, 9.0);
    double ly = plot_top + 10;
    for (unsigned si = 0; si < series_n; si++) {
        GraphSeries *s = graph->series->list[si];
        if (!s) continue;

        double rr, gg, bb;
        curve_color(si, &rr, &gg, &bb);

        cairo_set_source_rgb(cr, rr, gg, bb);
        cairo_rectangle(cr, plot_left + 5, ly - 7, 10, 3);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, 0, 0, 0);
        char buf[160];
        snprintf(buf, sizeof(buf), "%s%s%s",
            s->label ? s->label : "",
            (s->unit && s->label) ? " (" : (s->unit ? "(" : ""),
            s->unit ? s->unit : ""
        );
        if (s->unit) {
            size_t n = strlen(buf);
            if (n + 1 < sizeof(buf)) buf[n] = ')', buf[n + 1] = 0;
        }
        cairo_move_to(cr, plot_left + 20, ly - 5);
        cairo_show_text(cr, buf);

        ly += 12;
    }

    pthread_mutex_unlock(&graphs_mutex);
    return FALSE;
}

static GtkWidget *metric_picker_dialog_new(GtkWindow *parent, GtkComboBoxText **out_combo) {
    GtkWidget *d = gtk_dialog_new_with_buttons(
        "Add curve", parent, GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL, "Add", GTK_RESPONSE_OK, NULL
    );
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(d));
    GtkWidget *combo = gtk_combo_box_text_new();
    *out_combo = GTK_COMBO_BOX_TEXT(combo);

    gtk_combo_box_text_append(*out_combo, NULL, "Speed");
    gtk_combo_box_text_append(*out_combo, NULL, "Coolant Temperature");
    gtk_combo_box_text_append(*out_combo, NULL, "Intake Air Temperature");
    gtk_combo_box_text_append(*out_combo, NULL, "Intake Air Manifold Pressure");
    gtk_combo_box_text_append(*out_combo, NULL, "Intake Air MAF Rate");
    gtk_combo_box_text_append(*out_combo, NULL, "Engine Speed");
    gtk_combo_box_text_append(*out_combo, NULL, "Fuel Pressure");
    gtk_combo_box_text_append(*out_combo, NULL, "Fuel Level");
    gtk_combo_box_text_append(*out_combo, NULL, "Fuel ethanol");
    gtk_combo_box_text_append(*out_combo, NULL, "Fuel Rail Pressure");
    gtk_combo_box_text_append(*out_combo, NULL, "Fuel rate");
    gtk_combo_box_text_append(*out_combo, NULL, "Fuel trim long term bank1");
    gtk_combo_box_text_append(*out_combo, NULL, "Fuel trim long term bank2");
    gtk_combo_box_text_append(*out_combo, NULL, "Fuel trim short term bank1");
    gtk_combo_box_text_append(*out_combo, NULL, "Fuel trim short term bank2");
    gtk_combo_box_text_append(*out_combo, NULL, "Injection timing");
    gtk_combo_box_text_append(*out_combo, NULL, "Injection timing advance before TDC");
    for (int sensor_i = 1; sensor_i <= 8; sensor_i++) {
        char t[64];
        snprintf(t, sizeof(t), "Oxygen sensor %d voltage", sensor_i); gtk_combo_box_text_append(*out_combo, NULL, t);
        snprintf(t, sizeof(t), "Oxygen sensor %d current", sensor_i); gtk_combo_box_text_append(*out_combo, NULL, t);
        snprintf(t, sizeof(t), "Oxygen sensor %d air fuel equivalence ratio", sensor_i); gtk_combo_box_text_append(*out_combo, NULL, t);
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_container_add(GTK_CONTAINER(content), combo);
    gtk_widget_show_all(d);
    return d;
}

static void graphs_add_curve_clicked(GtkButton *btn, gpointer unused) {
    pthread_mutex_lock(&graphs_mutex);
    Graph *graph = g_object_get_data(G_OBJECT(btn), "graph_ptr");
    pthread_mutex_unlock(&graphs_mutex);
    if (!graph) return;

    GtkComboBoxText *combo = NULL;
    GtkWidget *d = metric_picker_dialog_new(GTK_WINDOW(gui->window), &combo);
    int resp = gtk_dialog_run(GTK_DIALOG(d));
    if (resp == GTK_RESPONSE_OK) {
        const char *sel = gtk_combo_box_text_get_active_text(combo);
        if (sel) {
            int arg = 0;
            MetricType t = metric_from_title(sel, &arg);
            pthread_mutex_lock(&graphs_mutex);
            ad_list_GraphSeries_append(graph->series, graph_series_new(t, arg));
            pthread_mutex_unlock(&graphs_mutex);
        }
    }
    gtk_widget_destroy(d);
}

static void graphs_remove_curve_clicked(GtkButton *btn, gpointer unused) {
    pthread_mutex_lock(&graphs_mutex);
    Graph *graph = g_object_get_data(G_OBJECT(btn), "graph_ptr");
    if (graph && graph->series) {
        unsigned idx = graph->series->size - 1;
        GraphSeries *s = graph->series->list[idx];
        graph->series->size--;
        if (s) {
            free(s->label);
            free(s->unit);
            ad_list_GraphData_free(s->data);
            free(s);
        }
        g_idle_add(graph_refresh_gsource, graph);
    }
    pthread_mutex_unlock(&graphs_mutex);
}

static void graphs_reset_curve_data_clicked(GtkButton *btn, gpointer unused) {
    pthread_mutex_lock(&graphs_mutex);
    Graph *graph = g_object_get_data(G_OBJECT(btn), "graph_ptr");
    if (graph && graph->series) {
        for (unsigned si = 0; si < graph->series->size; si++) {
            GraphSeries *s = graph->series->list[si];
            if (!s) continue;
            ad_list_GraphData_free(s->data);
            s->data = ad_list_GraphData_new();
        }
        graph_time_ms_reset(graph);
        g_idle_add(graph_refresh_gsource, graph);
    }
    pthread_mutex_unlock(&graphs_mutex);
}
static void graphs_remove_graph_clicked(GtkButton *btn, gpointer unused) {
    pthread_mutex_lock(&graphs_mutex);

    Graph *graph = g_object_get_data(G_OBJECT(btn), "graph_ptr");
    GtkWidget *graph_box = g_object_get_data(G_OBJECT(btn), "graph_box");
    if (!graph || !graph_box) {
        pthread_mutex_unlock(&graphs_mutex);
        return;
    }

    if (graphs) {
        for (unsigned i = 0; i < graphs->size; i++) {
            if (graphs->list[i] == graph) {
                for (unsigned j = i + 1; j < graphs->size; j++)
                    graphs->list[j - 1] = graphs->list[j];
                graphs->size--;
                break;
            }
        }
    }

    pthread_mutex_unlock(&graphs_mutex);

    gtk_widget_destroy(graph_box);
}

static void graph_attach_controls(GtkWidget *container, Graph *graph) {
    GtkWidget *h = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *b_add = gtk_button_new_with_label("Add curve");
    GtkWidget *b_rm  = gtk_button_new_with_label("Remove curve");
    GtkWidget *b_rst = gtk_button_new_with_label("Reset data");
    GtkWidget *b_del = gtk_button_new_with_label("Remove graph");

    g_object_set_data(G_OBJECT(b_add), "graph_ptr", graph);
    g_object_set_data(G_OBJECT(b_rm),  "graph_ptr", graph);
    g_object_set_data(G_OBJECT(b_rst), "graph_ptr", graph);
    g_object_set_data(G_OBJECT(b_del), "graph_ptr", graph);

    g_object_set_data(G_OBJECT(b_add), "graph_box", container);
    g_object_set_data(G_OBJECT(b_rm),  "graph_box", container);
    g_object_set_data(G_OBJECT(b_rst), "graph_box", container);
    g_object_set_data(G_OBJECT(b_del), "graph_box", container);

    g_signal_connect(G_OBJECT(b_add), "clicked", G_CALLBACK(graphs_add_curve_clicked), NULL);
    g_signal_connect(G_OBJECT(b_rm),  "clicked", G_CALLBACK(graphs_remove_curve_clicked), NULL);
    g_signal_connect(G_OBJECT(b_rst), "clicked", G_CALLBACK(graphs_reset_curve_data_clicked), NULL);
    g_signal_connect(G_OBJECT(b_del), "clicked", G_CALLBACK(graphs_remove_graph_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(h), b_add, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(h), b_rm,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(h), b_rst, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(h), b_del, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(container), h, FALSE, FALSE, 0);
}

static void *graphs_add_daemon(void *arg) {
    pthread_mutex_lock(&graphs_mutex);

    static int graph_count = 0;

    int active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(gui->graphs.list));
    if (0 <= active_index) {
        final char *activeGraph = gtk_combo_box_text_get_active_text(gui->graphs.list);
        if (!activeGraph) {
            pthread_mutex_unlock(&graphs_mutex);
            return null;
        }

        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
        GtkWidget *drawing_area = gtk_drawing_area_new();
        gtk_widget_set_size_request(drawing_area, 300, 300);
        gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);

        Graph *graph = graph_new(drawing_area, activeGraph, "");
        int argi = 0;
        MetricType t = metric_from_title(activeGraph, &argi);
        ad_list_GraphSeries_append(graph->series, graph_series_new(t, argi));
        ad_list_Graph_append(graphs, graph);

        graph_attach_controls(vbox, graph);

        assert(0 != g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(graphs_on_draw), strdup(activeGraph)));

        final int col = graph_count % 2;
        final int row = graph_count / 2;

        gtk_grid_attach(GTK_GRID(gui->graphs.container), vbox, col, row, 1, 1);
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

static void *graphs_reset_data_daemon(void *arg) {
    pthread_mutex_lock(&graphs_mutex);

    if (graphs) {
        for (unsigned gi = 0; gi < graphs->size; gi++) {
            Graph *g = graphs->list[gi];
            if (!g || !g->series) continue;
            for (unsigned si = 0; si < g->series->size; si++) {
                GraphSeries *s = g->series->list[si];
                if (!s) continue;
                ad_list_GraphData_free(s->data);
                s->data = ad_list_GraphData_new();
            }
        }
        ad_list_Graph_time_ms_reset(graphs);
    }

    pthread_mutex_unlock(&graphs_mutex);
    return null;
}

static void graphs_reset_data() {
    pthread_t t;
    pthread_create(&t, null, &graphs_reset_data_daemon, null);
}

static void refresh_changed(GtkCheckMenuItem *checkmenuitem, gpointer user_data) {
    bool state = gtk_check_menu_item_get_active(checkmenuitem);
    config.vehicleExplorer.autoRefresh = state;
    config_store();
    if (state) {
        refresh_dynamic();
    } else {
        THREAD_CANCEL(refresh_dynamic_thread);
    }
}

static gboolean expander_show_child(gpointer data) {
    GtkWidget *widget = (GtkWidget*)data;
    if (!gtk_widget_get_realized(widget)) gtk_widget_realize(widget);
    gtk_widget_set_visible(widget, true);
    gtk_widget_queue_draw(widget);
    return false;
}

static void expander_show_childs(GtkWidget *widget, gpointer data) {
    final const char *name = gtk_widget_get_name(widget);
    if (strcmp(name, "GtkBox") == 0 || strcmp(name, "GtkGrid") == 0) {
        gtk_container_foreach((GtkContainer*)widget, expander_show_childs, null);
    } else {
        g_idle_add(expander_show_child, widget);
    }
}

static void expander_activate(GtkExpander *expander, gpointer data) {
    final bool state = gtk_expander_get_expanded(expander);
    final const char * expander_name = gtk_expander_get_label(expander);
    if ( strcasecmp("graphs", expander_name) == 0 ) {
        graphs_should_refresh = !state;
    }
    if (!state) {
        gtk_container_foreach((GtkContainer*)expander, expander_show_childs, null);
    }
}

static void expanders_default_state(GtkWidget *widget, gpointer data) {
    final const char *name = gtk_widget_get_name(widget);
    if (strcmp(name, "GtkExpander") == 0) {
        assert(0 != g_signal_connect(G_OBJECT(widget), "activate", G_CALLBACK(expander_activate), NULL));
        gtk_container_foreach(
            GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(widget))),
            expanders_default_state, null
        );
    } else if (strcmp(name, "GtkBox") == 0 || strcmp(name, "GtkGrid") == 0) {
        gtk_container_foreach(GTK_CONTAINER(widget), expanders_default_state, null);
    } else {
        gtk_widget_set_visible(widget, false);
    }
}

#define VH_RETRIEVE_OX_SENSOR(i) \
    .sensor_##i = { \
        .voltage = GTK_LABEL(gtk_builder_get_object(builder, "vehicle-explorer-engine-sensors-"#i"-voltage")), \
        .current = GTK_LABEL(gtk_builder_get_object(builder, "vehicle-explorer-engine-sensors-"#i"-current")), \
        .ratio   = GTK_LABEL(gtk_builder_get_object(builder, "vehicle-explorer-engine-sensors-"#i"-ratio")), \
    }

static void generic_error_feedback_ok() {
    gtk_widget_hide_on_main_thread(GTK_WIDGET(gui->genericErrorFeedback));
}
static void hide() {
    THREAD_CANCEL(refresh_dynamic_thread);
    gtk_widget_hide_on_main_thread(gui->window);
}
static void init(final GtkBuilder *builder) {
    if (gui != null) {
        log_msg(LOG_DEBUG, "module already initialized");
        return;
    }

    graphs = ad_list_Graph_new();

    vehicleExplorerGui g = {
        .window = GTK_WIDGET(gtk_builder_get_object(builder, "window-vehicle-explorer")),
        .refreshIcon = (GtkSpinner*)gtk_builder_get_object(builder, "window-vehicle-explorer-global-refresh"),
        .errorFeedback = ERROR_FEEDBACK_RETRIEVE_WINDOWS(builder),
        .menuBar = {
            .autoRefresh = (GtkCheckMenuItem*)gtk_builder_get_object(builder, "vehicle-explorer-menubar-autorefresh"),
            .freeze_frame_thread = null,
            .freeze_frame_error_popup = GTK_WINDOW(gtk_builder_get_object(builder, "vehicle-explorer-freeze-frame-error")),
            .showFreezeFrame = GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder, "vehicle-explorer-menubar-show-freezeframe")),
            .data = {
                .all = GTK_MENU_ITEM(gtk_builder_get_object(builder, "vehicle-explorer-menubar-data-source-all")),
                .filter_by = GTK_WIDGET(gtk_builder_get_object(builder, "vehicle-explorer-menubar-data-source-filter-by"))
            }
        },
        .engine = {
            .expandableSection = (GtkBox*)gtk_builder_get_object(builder, "vehicle-explorer-expandable-section"),
            .load = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "vehicle-explorer-engine-load")),
            .speed = (GtkProgressBar*)gtk_builder_get_object(builder, "vehicle-explorer-engine-speed-graph"),
            .type = (GtkLabel*)gtk_builder_get_object(builder, "vehicle-explorer-engine-type"),
            .vehicleSpeed = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "vehicle-explorer-vehicle-speed")),
            .secondsSinceStart = GTK_LABEL(gtk_builder_get_object(builder, "vehicle-explorer-engine-seconds-since-start")),
            .intakeAir = {
                .temperature = (GtkProgressBar*)gtk_builder_get_object(builder, "vehicle-explorer-intake-air-temperature"),
                .manifoldPressure = (GtkProgressBar*)gtk_builder_get_object(builder, "vehicle-explorer-intake-air-manifold-pressure"),
                .mafRate = (GtkProgressBar*)gtk_builder_get_object(builder, "vehicle-explorer-intake-air-maf-rate")
            },
            .fuel = {
                .pressure = (GtkProgressBar*)gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-pressure"),
                .level = (GtkProgressBar*)gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-level"),
                .type = GTK_LABEL(gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-type")),
                .ethanol = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-ethanol")),
                .rate = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-rate")),
                .trim = {
                    .longTerm = {
                        .bank1 = (GtkProgressBar*)gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-trim-long-term-1"),
                        .bank2 = (GtkProgressBar*)gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-trim-long-term-2")
                    },
                    .shortTerm = {
                        .bank1 = (GtkProgressBar*)gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-trim-short-term-1"),
                        .bank2 = (GtkProgressBar*)gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-trim-short-term-2")
                    }
                },
                .rail = {
                    .pressure = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-rail-pressure"))
                },
                .status = GTK_LABEL(gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-system-status"))
            },
            .injectionSystem = {
                .injectionTiming = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "vehicle-explorer-engine-fuel-injection-timing")),
                .timingAdvance = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "vehicle-explorer-engine-time-advance"))
            },
            .coolant = {
                .temperature = (GtkProgressBar*)gtk_builder_get_object(builder, "vehicle-explorer-coolant-temperature-graph")
            },
            .ecu = {
                .voltage = (GtkLabel*)gtk_builder_get_object(builder, "vehicle-explorer-ecu-voltage"),
                .obdStandard = (GtkLabel*)gtk_builder_get_object(builder, "vehicle-explorer-ecu-obd-standard")
            },
            .oxSensors = {
                VH_RETRIEVE_OX_SENSOR(1),VH_RETRIEVE_OX_SENSOR(2),VH_RETRIEVE_OX_SENSOR(3),VH_RETRIEVE_OX_SENSOR(4),
                VH_RETRIEVE_OX_SENSOR(5),VH_RETRIEVE_OX_SENSOR(6),VH_RETRIEVE_OX_SENSOR(7),VH_RETRIEVE_OX_SENSOR(8)
            },
            .tests = GTK_CONTAINER(gtk_builder_get_object(builder, "vehicle-explorer-engine-tests"))
        },
        .graphs = {
            .add = GTK_BUTTON(gtk_builder_get_object(builder, "vehicle-explorer-graphs-add")),
            .list = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "vehicle-explorer-graphs-list")),
            .container = GTK_GRID(gtk_builder_get_object(builder, "vehicle-explorer-graphs-container")),
            .resetData = GTK_BUTTON(gtk_builder_get_object(builder, "vehicle-explorer-graphs-reset-data"))
        },
        .genericErrorFeedback = GTK_MESSAGE_DIALOG(gtk_builder_get_object(builder, "vehicle-explorer-feedback-info"))
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
    for (int sensor_i = 1; sensor_i <= 8; sensor_i++) {
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

    counter_init_with(GTK_WIDGET(g.engine.vehicleSpeed), "counter_85_2_255_0_0_255.png");
    counter_init_with(GTK_WIDGET(g.engine.speed), "counter_85_2_255_0_0_255.png");
    counter_init_with(GTK_WIDGET(g.engine.fuel.pressure), "gaugehalf_225_5_255_0_0_255.png");
    counter_init_with(GTK_WIDGET(g.engine.fuel.rail.pressure), "gaugehalf_225_5_255_0_0_255.png");

    expanders_default_state((GtkWidget*)gui->engine.expandableSection, null);

    assert(0 != g_signal_connect(G_OBJECT(gui->window), "delete-event", G_CALLBACK(hide), NULL));
    assert(0 != g_signal_connect(G_OBJECT(g.menuBar.freeze_frame_error_popup), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), null));

    error_feedback_windows_init(gui->errorFeedback);

    assert(0 != g_signal_connect(G_OBJECT(gui->genericErrorFeedback), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), null));
    gtk_builder_add_callback_symbol(builder, "vehicle-explorer-generic-error-feedback-ok", &generic_error_feedback_ok);
    gtk_builder_add_callback_symbol(builder, "vehicle-explorer-graphs-reset-data", &graphs_reset_data);
    gtk_builder_add_callback_symbol(builder, "vehicle-explorer-graphs-add", &graphs_add);
    gtk_builder_add_callback_symbol(builder, "window-vehicle-explorer-global-refresh-click", &refresh_one_time_with_spinner);
    gtk_builder_add_callback_symbol(builder, "window-vehicle-explorer-global-refresh-changed", G_CALLBACK(&refresh_changed));
    gtk_builder_add_callback_symbol(builder, "vehicle-explorer-freeze-frame-error-ok", G_CALLBACK(&freeze_frame_error_ok));
    gtk_builder_add_callback_symbol(builder, "window-vehicle-explorer-data-freeze-frame", &data_freeze_frame);

    MENUBAR_DATA_CONNECT()
}

static void end() {
    if ( gui != null ) {
        free(gui);
        gui = null;
    }
}

static void show() {
    gtk_window_show_ensure_ontop(gui->window);
    gtk_check_menu_item_set_active(gui->menuBar.autoRefresh, config.vehicleExplorer.autoRefresh);
    refresh_one_time();
    refresh_changed(gui->menuBar.autoRefresh, null);
}

mod_gui * mod_gui_vehicle_explorer_new() {
    mod_gui * mg = (mod_gui*)malloc(sizeof(mod_gui));
    mg->init = init;
    mg->end = end;
    mg->name = strdup("Vehicle Explorer");
    mg->show = show;
    mg->hide = hide;
    return mg;
}