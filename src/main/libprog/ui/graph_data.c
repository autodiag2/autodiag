#include "libprog/ui/graph_data.h"

double graph_time_start_ms = -1;
double graph_time_ms_now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double now_ms = tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
    return now_ms;
}
void ad_list_Graph_time_ms_reset(ad_list_Graph * graphs) {
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

GraphSeries * graph_series_new(ad_object_vehicle_signal* signal) {
    GraphSeries *s = malloc(sizeof(*s));
    s->signal = signal;
    s->data = ad_list_GraphData_new();
    return s;
}
Graph * graph_new(GtkWidget *drawing_area, char *title, char *unit_unused) {
    Graph *g = malloc(sizeof(*g));
    g->widget = drawing_area;
    g->title = strdup(title);
    g->series = ad_list_GraphSeries_new();
    g->time_start_ms = -1;
    return g;
}
int Graph_cmp(Graph*g1, Graph*g2) {
    return g1 - g2;
}
AD_LIST_SRC(Graph)

Graph * ad_list_Graph_get_by_title(ad_list_Graph * list, char * title) {
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

bool ad_list_Graph_append_data(ad_list_Graph * list, char * title, double data) {
    log_msg(LOG_ERROR, "TODO");
    return true;
}