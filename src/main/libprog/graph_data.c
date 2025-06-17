#include "libprog/graph_data.h"

GraphData * graph_data_new(final double data) {
    GraphData * graph_data = (GraphData*)malloc(sizeof(GraphData));
    graph_data->time = time(NULL);
    graph_data->data = data;
    return graph_data;
}

LIST_DEFINE_MEMBERS_SYM_AUTO(GraphData)

Graph * graph_new(char *title, char *unit) {
    Graph * graph = (Graph*)malloc(sizeof(Graph));
    graph->data = GraphData_list_new();
    graph->title = strdup(title);
    graph->unit = strdup(unit);
    return graph;
}
void graph_append(Graph *graph, double data) {
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