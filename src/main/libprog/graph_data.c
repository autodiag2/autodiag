#include "libprog/graph_data.h"

double graph_time_start_ms = 0;

GraphData * graph_data_new(final double data) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double now_ms = tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;

    if (graph_time_start_ms == 0) graph_time_start_ms = now_ms;
    int time_ms = now_ms - graph_time_start_ms;

    GraphData * graph_data = (GraphData*)malloc(sizeof(GraphData));
    graph_data->time = time_ms;
    graph_data->data = data;
    return graph_data;
}

LIST_SRC(GraphData)

Graph * graph_new(GtkWidget * widget, char *title, char *unit) {
    Graph * graph = (Graph*)malloc(sizeof(Graph));
    graph->data = GraphData_list_new();
    graph->title = strdup(title);
    graph->widget = widget;
    graph->unit = strdup(unit);
    return graph;
}
void graph_append_data(Graph *graph, double data) {
    assert(graph != null);
    GraphData_list_append(graph->data, graph_data_new(data));
}

LIST_SRC(Graph)

Graph * Graph_list_get_by_title(Graph_list * list, char * title) {
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

bool Graph_list_append_data(Graph_list * list, char * title, double data) {
    Graph * graph = Graph_list_get_by_title(list, title);
    if ( graph == null ) {
        return false;
    } else {
        GraphData_list_append(graph->data, graph_data_new(data));
        return true;
    }
}