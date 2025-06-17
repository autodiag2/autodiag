#include "libprog/graph_data.h"

GraphData * graph_data_new(final double data) {
    GraphData * graph_data = (GraphData*)malloc(sizeof(GraphData));
    graph_data->time = time(NULL);
    graph_data->data = data;
    return graph_data;
}

LIST_DEFINE_MEMBERS_SYM_AUTO(GraphData)

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

LIST_DEFINE_MEMBERS_SYM_AUTO(Graph)

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