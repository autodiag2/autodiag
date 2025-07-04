#ifndef __GRAPH_DATA_H
#define __GRAPH_DATA_H

#include <time.h>
#include "libautodiag/lib.h"
#include "ui/gtk.h"

typedef struct {
    double data;
    int time;
} GraphData;

LIST_H(GraphData)

typedef struct {
    char *title;
    GraphData_list * data;
    char *unit;
    GtkWidget * widget;
} Graph;
LIST_H(Graph)
extern double graph_time_start_ms;

GraphData * graph_data_new(final double data);
Graph * graph_new(GtkWidget * widget, char *title, char *unit);
void graph_append_data(Graph *graph, double data);
Graph * Graph_list_get_by_title(Graph_list * list, char * title);
bool Graph_list_append_data(Graph_list * list, char * title, double data);

#endif