#ifndef __GRAPH_DATA_H
#define __GRAPH_DATA_H

#include <time.h>
#include "libautodiag/lib.h"

typedef struct {
    double data;
    time_t time;
} GraphData;

LIST_DEFINE_WITH_MEMBERS_AUTO(GraphData)

typedef struct {
    char *title;
    GraphData_list * data;
    char *unit;
} Graph;
LIST_DEFINE_WITH_MEMBERS_AUTO(Graph)

GraphData * graph_data_new(final double data);
Graph * graph_new(char *title, char *unit);
void graph_append(Graph *graph, double data);
Graph * Graph_list_get_by_title(Graph_list * list, char * title);

#endif