#ifndef __GRAPH_DATA_H
#define __GRAPH_DATA_H

#include <time.h>
#include "libautodiag/lib.h"
#include "libprog/ui/gtk.h"

typedef struct {
    double data;
    int time;
} GraphData;

AD_LIST_H(GraphData)

typedef enum {
    METRIC_SPEED,
    METRIC_COOLANT_TEMP,
    METRIC_INTAKE_AIR_TEMP,
    METRIC_INTAKE_MANIFOLD_PRESSURE,
    METRIC_MAF_RATE,
    METRIC_ENGINE_SPEED,
    METRIC_FUEL_PRESSURE,
    METRIC_FUEL_LEVEL,
    METRIC_FUEL_ETHANOL,
    METRIC_FUEL_RAIL_PRESSURE,
    METRIC_FUEL_RATE,
    METRIC_FUEL_TRIM_LT_B1,
    METRIC_FUEL_TRIM_LT_B2,
    METRIC_FUEL_TRIM_ST_B1,
    METRIC_FUEL_TRIM_ST_B2,
    METRIC_INJECTION_TIMING,
    METRIC_INJECTION_ADV_BTDC,
    METRIC_OX_VOLTAGE,
    METRIC_OX_CURRENT,
    METRIC_OX_RATIO,
} MetricType;

typedef struct {
    MetricType type;
    int arg;                 // oxygen sensor index 1..8 (otherwise 0)
    char *label;             // curve label
    char *unit;              // curve unit (for legend only)
    list_GraphData *data;    // per-curve time series
} GraphSeries;

AD_LIST_H(GraphSeries)

typedef struct {
    GtkWidget *widget;       // drawing area
    char *title;             // graph title
    list_GraphSeries *series;
    double time_start_ms;
} Graph;

AD_LIST_H(Graph)

extern double graph_time_start_ms;

Graph * graph_new(GtkWidget * widget, char *title, char *unit);
Graph * list_Graph_get_by_title(list_Graph * list, char * title);
bool list_Graph_append_data(list_Graph * list, char * title, double data);
double graph_time_ms_now();
void graph_time_ms_reset(Graph *g);
double graph_time_ms_ellapsed(Graph *g);
void list_Graph_time_ms_reset(list_Graph * graphs);
GraphSeries * graph_series_new(MetricType t, int arg);

#endif