#ifndef __AD_GRAPH_DATA_H
#define __AD_GRAPH_DATA_H

#include <time.h>
#include "libautodiag/lib.h"
#include "libprog/ui/gtk.h"
#include "libautodiag/com/vehicle_signal.h"

typedef struct {
    double y_value;
    double x_value;
} GraphData;

AD_LIST_H(GraphData)

typedef struct {
    ad_object_vehicle_signal * signal;
    ad_list_GraphData *data;    // per-curve time series
    char * label;               // Custom label if null, use the signal
    char * x_unit;              // default to time
    char * y_unit;              // default to signal->unit
} GraphSeries;

AD_LIST_H(GraphSeries)

typedef struct {
    GtkWidget *widget;       // drawing area
    char *title;             // graph title
    ad_list_GraphSeries *series;
    double time_start_ms;
} Graph;

AD_LIST_H(Graph)

extern double graph_time_start_ms;

Graph * graph_new(GtkWidget * widget, char *title);
Graph * ad_list_Graph_get_by_title(ad_list_Graph * list, char * title);
bool ad_list_Graph_append_data(ad_list_Graph * list, char * title, double y_value);
double graph_time_ms_now();
void graph_time_ms_reset(Graph *g);
double graph_time_ms_ellapsed(Graph *g);
void ad_list_Graph_time_ms_reset(ad_list_Graph * graphs);
GraphSeries * graph_series_new(ad_object_vehicle_signal* signal);

#endif