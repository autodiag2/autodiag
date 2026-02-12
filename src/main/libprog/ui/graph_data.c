#include "libprog/ui/graph_data.h"

double graph_time_start_ms = -1;
double graph_time_ms_now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double now_ms = tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
    return now_ms;
}
void list_Graph_time_ms_reset(list_Graph * graphs) {
    for(int i = 0; i < graphs->size; i++) {
        graph_time_ms_reset(graphs->list[i]);
    }
}
double graph_time_ms_ellapsed(Graph *g) {
    double now_ms = graph_time_ms_now();
    if ( g->time_start_ms < 0 ) {
        g->time_start_ms = now_ms;
        return 0;
    } else {
        return now_ms - g->time_start_ms;
    }
}
void graph_time_ms_reset(Graph *g) {
    g->time_start_ms = -1;
}

int GraphData_cmp(GraphData*g1, GraphData*g2) {
    return g1 - g2;
}
AD_LIST_SRC(GraphData)
int GraphSeries_cmp(GraphSeries*g1, GraphSeries*g2) {
    return g1 - g2;
}
AD_LIST_SRC(GraphSeries)
static void metric_info(MetricType t, int arg, const char **label, const char **unit) {
    switch (t) {
        case METRIC_SPEED: *label="Speed"; *unit="km/h"; return;
        case METRIC_COOLANT_TEMP: *label="Coolant Temperature"; *unit="°C"; return;
        case METRIC_INTAKE_AIR_TEMP: *label="Intake Air Temperature"; *unit="°C"; return;
        case METRIC_INTAKE_MANIFOLD_PRESSURE: *label="Intake Air Manifold Pressure"; *unit="kPa"; return;
        case METRIC_MAF_RATE: *label="Intake Air MAF Rate"; *unit="g/s"; return;
        case METRIC_ENGINE_SPEED: *label="Engine Speed"; *unit="r/min"; return;
        case METRIC_FUEL_PRESSURE: *label="Fuel Pressure"; *unit="kPa"; return;
        case METRIC_FUEL_LEVEL: *label="Fuel Level"; *unit="%"; return;
        case METRIC_FUEL_ETHANOL: *label="Fuel ethanol"; *unit="%"; return;
        case METRIC_FUEL_RAIL_PRESSURE: *label="Fuel Rail Pressure"; *unit="kPa"; return;
        case METRIC_FUEL_RATE: *label="Fuel rate"; *unit="L/h"; return;
        case METRIC_FUEL_TRIM_LT_B1: *label="Fuel trim long term bank1"; *unit="%"; return;
        case METRIC_FUEL_TRIM_LT_B2: *label="Fuel trim long term bank2"; *unit="%"; return;
        case METRIC_FUEL_TRIM_ST_B1: *label="Fuel trim short term bank1"; *unit="%"; return;
        case METRIC_FUEL_TRIM_ST_B2: *label="Fuel trim short term bank2"; *unit="%"; return;
        case METRIC_INJECTION_TIMING: *label="Injection timing"; *unit="°"; return;
        case METRIC_INJECTION_ADV_BTDC: *label="Injection timing advance before TDC"; *unit="°"; return;
        case METRIC_OX_VOLTAGE: *label="Oxygen sensor voltage"; *unit="V"; return;
        case METRIC_OX_CURRENT: *label="Oxygen sensor current"; *unit="mA"; return;
        case METRIC_OX_RATIO: *label="Oxygen sensor air fuel equivalence ratio"; *unit="ratio"; return;
    }
    *label = "Unknown";
    *unit = "";
}
GraphSeries * graph_series_new(MetricType t, int arg) {
    GraphSeries *s = malloc(sizeof(*s));
    const char *lbl = 0, *unit = 0;
    metric_info(t, arg, &lbl, &unit);
    s->type = t;
    s->arg = arg;
    s->label = strdup(lbl);
    s->unit = strdup(unit);
    s->data = list_GraphData_new();
    return s;
}
Graph * graph_new(GtkWidget *drawing_area, char *title, char *unit_unused) {
    Graph *g = malloc(sizeof(*g));
    g->widget = drawing_area;
    g->title = strdup(title);
    g->series = list_GraphSeries_new();
    g->time_start_ms = -1;
    return g;
}
int Graph_cmp(Graph*g1, Graph*g2) {
    return g1 - g2;
}
AD_LIST_SRC(Graph)

Graph * list_Graph_get_by_title(list_Graph * list, char * title) {
    assert(list != null);
    assert(title != null);
    for(int i = 0; i < list->size; i++) {
        Graph * graph = list->list[i];
        assert(graph != null);
        assert(graph->title != null);
        if ( strcmp(graph->title, title) == 0 ) {
            return graph;
        }
    }
    return null;
}

bool list_Graph_append_data(list_Graph * list, char * title, double data) {
    log_msg(LOG_ERROR, "TODO");
    return true;
}