#include "ui/dyno.h"

static DynoGui *gui = null;

static pthread_t *dyno_thread = null;
static pthread_mutex_t dyno_mutex = PTHREAD_MUTEX_INITIALIZER;

static gboolean dyno_running = false;
static gboolean dyno_request_stop = false;

static ad_list_Graph *dyno_graphs = null;
/*
typedef struct {
    double t_ms;
    double speed_kmh;
    double rpm;
} DynoSample;

typedef struct {
    DynoSample *v;
    unsigned n;
    unsigned cap;
} DynoSamples;

static DynoSamples dyno_samples = {0};
static double peak_time = 0;
static double peak_speed = 0;
static double peak_rpm = 0;
static double peak_kw = 0;
static double peak_hp = 0;
static double peak_tq = 0;
static unsigned dyno_run_index = (unsigned)-1;

MENUBAR_DATA_ALL_IN_ONE

#define AD_GRAPH_TITLE_SPEED "Speed(km/h) (time s)"
#define AD_GRAPH_TITLE_RPM "RPM (time s)"
#define AD_GRAPH_TITLE_POWER "Power(kW) (rpm)"
#define AD_GRAPH_TITLE_TORQUE "Torque(N.m) (rpm)"

static const char *unit_for_metric_title(const char *title) {
    if (strcmp(title, AD_GRAPH_TITLE_SPEED) == 0) return "km/h";
    if (strcmp(title, AD_GRAPH_TITLE_RPM) == 0) return "r/min";
    if (strcmp(title, AD_GRAPH_TITLE_POWER) == 0) return "kW";
    if (strcmp(title, AD_GRAPH_TITLE_TORQUE) == 0) return "N·m";
    return "";
}

static const char *xunit_for_metric_title(const char *title) {
    if (strcmp(title, AD_GRAPH_TITLE_SPEED) == 0) return "s";
    if (strcmp(title, AD_GRAPH_TITLE_RPM) == 0) return "s";
    if (strcmp(title, AD_GRAPH_TITLE_POWER) == 0) return "r/min";
    if (strcmp(title, AD_GRAPH_TITLE_TORQUE) == 0) return "r/min";
    return "";
}

static MetricType metric_type_for_graph_title(const char *title) {
    if (strcmp(title, AD_GRAPH_TITLE_RPM) == 0) return METRIC_ENGINE_SPEED;
    return METRIC_SPEED;
}

static void graph_display_title(const char *title, char *buf, size_t n) {
    snprintf(buf, n, "%s", title ? title : "");
}

static GraphSeries *dyno_graph_ensure_series(Graph *g, unsigned si, MetricType type) {
    if (!g) return null;
    if (!g->series) g->series = ad_list_GraphSeries_new();

    while (g->series->size <= si) {
        GraphSeries *s = graph_series_new(type, 0);
        if (!s) return null;

        if (s->label) free(s->label);
        if (s->unit) free(s->unit);

        char buf[64];
        snprintf(buf, sizeof(buf), "Run %u", g->series->size + 1);

        s->label = strdup(buf);
        s->unit = strdup(unit_for_metric_title(g->title));

        ad_list_GraphSeries_append(g->series, s);
    }

    GraphSeries *s = g->series->list[si];
    if (!s) return null;

    s->type = type;

    {
        char buf[64];
        snprintf(buf, sizeof(buf), "Run %u", si + 1);
        if (s->label) free(s->label);
        s->label = strdup(buf);
    }

    if (s->unit) free(s->unit);
    s->unit = strdup(unit_for_metric_title(g->title));

    if (!s->data) s->data = ad_list_GraphData_new();
    return s;
}

static void dyno_samples_clear() {
    if (dyno_samples.v) free(dyno_samples.v);
    dyno_samples.v = null;
    dyno_samples.n = 0;
    dyno_samples.cap = 0;

    peak_time = 0;
    peak_speed = 0;
    peak_rpm = 0;
    peak_kw = 0;
    peak_hp = 0;
    peak_tq = 0;
}

static void dyno_samples_push(double t_ms, double speed_kmh, double rpm) {
    if (dyno_samples.cap <= dyno_samples.n) {
        unsigned nc = dyno_samples.cap ? dyno_samples.cap * 2u : 512u;
        DynoSample *nv = realloc(dyno_samples.v, (size_t)nc * sizeof(*nv));
        if (!nv) return;
        dyno_samples.v = nv;
        dyno_samples.cap = nc;
    }
    dyno_samples.v[dyno_samples.n++] = (DynoSample){ .t_ms = t_ms, .speed_kmh = speed_kmh, .rpm = rpm };
}

static double dyno_now_ms() {
    struct timeval tv;
    gettimeofday(&tv, null);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
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
    unsigned k = (i - 2) % (sizeof(pal) / sizeof(pal[0]));
    *r = pal[k][0];
    *g = pal[k][1];
    *b = pal[k][2];
}

static gboolean graph_refresh_gsource(gpointer data) {
    Graph *g = (Graph*)data;
    if (g && g->widget) gtk_widget_queue_draw(g->widget);
    return false;
}

static gboolean dyno_graph_on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    pthread_mutex_lock(&dyno_mutex);

    char *graph_title = (char*)user_data;
    Graph *graph = dyno_graphs ? ad_list_Graph_get_by_title(dyno_graphs, graph_title) : null;
    if (!graph) {
        pthread_mutex_unlock(&dyno_mutex);
        return false;
    }

    GtkAllocation a;
    gtk_widget_get_allocation(widget, &a);
    int width = a.width;
    int height = a.height;

    int margin_left = 55;
    int margin_bottom = 32;
    int margin_top = 28;
    int margin_right = 12;

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 13.0);
    cairo_set_source_rgb(cr, 0, 0, 0);

    {
        char title_buf[128];
        cairo_text_extents_t te;
        graph_display_title(graph->title, title_buf, sizeof(title_buf));
        cairo_text_extents(cr, title_buf, &te);
        cairo_move_to(cr, (width - te.width) / 2 - te.x_bearing, margin_top - 10);
        cairo_show_text(cr, title_buf);
    }

    double min_val = 0;
    double max_val = 0;
    double min_x = 0;
    double max_x = 0;
    gboolean has = false;

    for (unsigned si = 0; graph->series && si < graph->series->size; si++) {
        GraphSeries *s = graph->series->list[si];
        if (!s || !s->data || s->data->size < 1) continue;

        for (unsigned i = 0; i < s->data->size; i++) {
            GraphData *d = s->data->list[i];
            if (!d) continue;

            if (!has) {
                min_val = max_val = d->data;
                min_x = max_x = d->time;
                has = true;
            } else {
                if (d->data < min_val) min_val = d->data;
                if (max_val < d->data) max_val = d->data;
                if (d->time < min_x) min_x = d->time;
                if (max_x < d->time) max_x = d->time;
            }
        }
    }

    if (!has) {
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 14.0);
        cairo_set_source_rgb(cr, 0.55, 0.55, 0.55);
        {
            const char *msg = "No data";
            cairo_text_extents_t te2;
            cairo_text_extents(cr, msg, &te2);
            cairo_move_to(cr, (width - te2.width) / 2 - te2.x_bearing,
                             (height + te2.height) / 2 - te2.y_bearing);
            cairo_show_text(cr, msg);
        }
        pthread_mutex_unlock(&dyno_mutex);
        return false;
    }

    if (min_val == max_val) {
        min_val -= 0.5;
        max_val += 0.5;
    }
    if (min_x == max_x) {
        min_x -= 1.0;
        max_x += 1.0;
    }

    double px_w = (double)(width - margin_left - margin_right);
    double px_h = (double)(height - margin_top - margin_bottom);

    double y_scale = px_h / (max_val - min_val);
    double x_scale = px_w / (max_x - min_x);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, margin_left, margin_top);
    cairo_line_to(cr, margin_left, height - margin_bottom);
    cairo_stroke(cr);

    cairo_move_to(cr, margin_left, height - margin_bottom);
    cairo_line_to(cr, width - margin_right, height - margin_bottom);
    cairo_stroke(cr);

    int ticks = 6;
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 9.0);

    cairo_set_source_rgb(cr, 0.85, 0.85, 0.85);
    cairo_set_line_width(cr, 0.6);

    for (int ti = 0; ti <= ticks; ti++) {
        double vx = min_x + (max_x - min_x) * ((double)ti / (double)ticks);
        double x = margin_left + (vx - min_x) * x_scale;

        cairo_move_to(cr, x, margin_top);
        cairo_line_to(cr, x, height - margin_bottom);
        cairo_stroke(cr);
    }

    for (int ti = 0; ti <= ticks; ti++) {
        double vy = min_val + (max_val - min_val) * ((double)ti / (double)ticks);
        double y = (double)(height - margin_bottom) - (vy - min_val) * y_scale;

        cairo_move_to(cr, margin_left, y);
        cairo_line_to(cr, width - margin_right, y);
        cairo_stroke(cr);
    }

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_font_size(cr, 9.0);

    for (int ti = 0; ti <= ticks; ti++) {
        double vx = min_x + (max_x - min_x) * ((double)ti / (double)ticks);
        double x = margin_left + (vx - min_x) * x_scale;

        cairo_move_to(cr, x, height - margin_bottom);
        cairo_line_to(cr, x, height - margin_bottom + 4);
        cairo_stroke(cr);

        {
            char buf[64];
            cairo_text_extents_t te3;
            snprintf(buf, sizeof(buf), "%.1f", vx);
            cairo_text_extents(cr, buf, &te3);
            cairo_move_to(cr, x - te3.width / 2, height - margin_bottom + 14);
            cairo_show_text(cr, buf);
        }
    }

    for (int ti = 0; ti <= ticks; ti++) {
        double v = min_val + (max_val - min_val) * ((double)ti / (double)ticks);
        double y = (double)(height - margin_bottom) - (v - min_val) * y_scale;

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 1.0);
        cairo_move_to(cr, margin_left - 4, y);
        cairo_line_to(cr, margin_left, y);
        cairo_stroke(cr);

        {
            char buf[64];
            cairo_text_extents_t tte;
            snprintf(buf, sizeof(buf), "%.2f", v);
            cairo_text_extents(cr, buf, &tte);
            cairo_move_to(cr, margin_left - 6 - tte.width, y + (tte.height / 2));
            cairo_show_text(cr, buf);
        }
    }

    for (unsigned si = 0; graph->series && si < graph->series->size; si++) {
        GraphSeries *s = graph->series->list[si];
        if (!s || !s->data || s->data->size < 2) continue;

        double rr, gg, bb;
        curve_color(si, &rr, &gg, &bb);
        cairo_set_source_rgb(cr, rr, gg, bb);
        cairo_set_line_width(cr, (si + 1 == graph->series->size) ? 1.8 : 1.3);

        {
            GraphData *d0 = s->data->list[0];
            double x0 = margin_left + (d0->time - min_x) * x_scale;
            double y0 = (double)(height - margin_bottom) - (d0->data - min_val) * y_scale;
            cairo_move_to(cr, x0, y0);
        }

        for (unsigned i = 1; i < s->data->size; i++) {
            GraphData *d = s->data->list[i];
            double x = margin_left + (d->time - min_x) * x_scale;
            double y = (double)(height - margin_bottom) - (d->data - min_val) * y_scale;
            cairo_line_to(cr, x, y);
        }
        cairo_stroke(cr);
    }

    cairo_set_font_size(cr, 9.0);
    {
        double ly = margin_top + 10;
        for (unsigned si = 0; graph->series && si < graph->series->size; si++) {
            GraphSeries *s = graph->series->list[si];
            if (!s || !s->data || s->data->size < 1) continue;

            double rr, gg, bb;
            curve_color(si, &rr, &gg, &bb);
            cairo_set_source_rgb(cr, rr, gg, bb);
            cairo_rectangle(cr, margin_left + 6, ly - 7, 10, 3);
            cairo_fill(cr);

            cairo_set_source_rgb(cr, 0, 0, 0);
            cairo_move_to(cr, margin_left + 22, ly - 5);
            cairo_show_text(cr, s->label ? s->label : "Run");
            ly += 12;
        }
    }

    pthread_mutex_unlock(&dyno_mutex);
    return false;
}

static void dyno_graph_add_series_point(Graph *g, unsigned si, double x, double y) {
    if (!g) return;

    GraphSeries *s = dyno_graph_ensure_series(g, si, metric_type_for_graph_title(g->title));
    if (!s) return;

    GraphData *d = malloc(sizeof(*d));
    if (!d) return;

    d->time = x;
    d->data = y;
    ad_list_GraphData_append(s->data, d);
}

static void dyno_graph_reset_series(Graph *g, unsigned si) {
    if (!g || !g->series || g->series->size <= si) return;
    GraphSeries *s = g->series->list[si];
    if (!s) return;
    if (s->data) ad_list_GraphData_free(s->data);
    s->data = ad_list_GraphData_new();
}

static void dyno_graph_reset(Graph *g) {
    if (!g || !g->series) return;
    for (unsigned si = 0; si < g->series->size; si++) {
        GraphSeries *s = g->series->list[si];
        if (!s) continue;
        if (s->data) ad_list_GraphData_free(s->data);
        s->data = ad_list_GraphData_new();
    }
}

static Graph *dyno_get_graph(char *title) {
    return dyno_graphs ? ad_list_Graph_get_by_title(dyno_graphs, title) : null;
}

static void dyno_update_labels_locked(double t_s, double speed, double rpm, double p_kw, double hp, double tq_nm) {
    if (gui->status.lbl_state) gtk_widget_printf(GTK_WIDGET(gui->status.lbl_state), "%s", dyno_running ? "Running" : "Idle");
    if (gui->status.lbl_time)  gtk_widget_printf(GTK_WIDGET(gui->status.lbl_time), "%.2f s", t_s);
    if (gui->status.lbl_speed) gtk_widget_printf(GTK_WIDGET(gui->status.lbl_speed), "%.1f km/h", speed);
    if (gui->status.lbl_rpm)   gtk_widget_printf(GTK_WIDGET(gui->status.lbl_rpm), "%.0f r/min", rpm);
    if (gui->status.lbl_pwr)   gtk_widget_printf(GTK_WIDGET(gui->status.lbl_pwr), "%.2f kW", p_kw);
    if (gui->status.lbl_hp)    gtk_widget_printf(GTK_WIDGET(gui->status.lbl_hp), "%.2f hp", hp);
    if (gui->status.lbl_tq)    gtk_widget_printf(GTK_WIDGET(gui->status.lbl_tq), "%.1f N·m", tq_nm);

    if (gui->peak.lbl_time)  gtk_widget_printf(GTK_WIDGET(gui->peak.lbl_time), "%.2f s", peak_time);
    if (gui->peak.lbl_speed) gtk_widget_printf(GTK_WIDGET(gui->peak.lbl_speed), "%.1f km/h", peak_speed);
    if (gui->peak.lbl_rpm)   gtk_widget_printf(GTK_WIDGET(gui->peak.lbl_rpm), "%.0f r/min", peak_rpm);
    if (gui->peak.lbl_pwr)   gtk_widget_printf(GTK_WIDGET(gui->peak.lbl_pwr), "%.2f kW", peak_kw);
    if (gui->peak.lbl_hp)    gtk_widget_printf(GTK_WIDGET(gui->peak.lbl_hp), "%.2f hp", peak_hp);
    if (gui->peak.lbl_tq)    gtk_widget_printf(GTK_WIDGET(gui->peak.lbl_tq), "%.1f N·m", peak_tq);
}

static gboolean dyno_ui_tick(gpointer unused) {
    pthread_mutex_lock(&dyno_mutex);

    if (dyno_samples.n < 1) {
        dyno_update_labels_locked(0, 0, 0, 0, 0, 0);
        pthread_mutex_unlock(&dyno_mutex);
        return true;
    }

    DynoSample s = dyno_samples.v[dyno_samples.n - 1];
    double t_s = (dyno_samples.v[dyno_samples.n - 1].t_ms - dyno_samples.v[0].t_ms) / 1000.0;

    double p_kw = 0;
    double tq_nm = 0;

    if (2 <= dyno_samples.n) {
        DynoSample a = dyno_samples.v[dyno_samples.n - 2];
        DynoSample b = dyno_samples.v[dyno_samples.n - 1];

        double dt = (b.t_ms - a.t_ms) / 1000.0;

        if (0.001 < dt) {
            double v0 = a.speed_kmh / 3.6;
            double v1 = b.speed_kmh / 3.6;
            double acc = (v1 - v0) / dt;
            double mass = gui->params.mass_kg ? gtk_spin_button_get_value(gui->params.mass_kg) : 1200.0;
            double force = mass * acc;
            double power_w = force * v1;
            double w_rad = (b.rpm * (2.0 * M_PI)) / 60.0;

            p_kw = power_w / 1000.0;
            if (1.0 < w_rad) tq_nm = power_w / w_rad;
        }
    }

    {
        double hp = p_kw * 1.341022;

        if (hp > peak_hp) {
            peak_hp = hp;
            peak_kw = p_kw;
            peak_tq = tq_nm;
            peak_speed = s.speed_kmh;
            peak_rpm = s.rpm;
            peak_time = t_s;
        }

        dyno_update_labels_locked(t_s, s.speed_kmh, s.rpm, p_kw, hp, tq_nm);
    }

    pthread_mutex_unlock(&dyno_mutex);
    return true;
}

static void dyno_build_results_from_samples() {
    pthread_mutex_lock(&dyno_mutex);

    Graph *g_speed = dyno_get_graph(AD_GRAPH_TITLE_SPEED);
    Graph *g_rpm   = dyno_get_graph(AD_GRAPH_TITLE_RPM);
    Graph *g_pwr   = dyno_get_graph(AD_GRAPH_TITLE_POWER);
    Graph *g_tq    = dyno_get_graph(AD_GRAPH_TITLE_TORQUE);

    if (!g_speed || !g_rpm || !g_pwr || !g_tq) {
        pthread_mutex_unlock(&dyno_mutex);
        return;
    }

    dyno_graph_ensure_series(g_speed, dyno_run_index, METRIC_SPEED);
    dyno_graph_ensure_series(g_rpm, dyno_run_index, METRIC_ENGINE_SPEED);
    dyno_graph_ensure_series(g_pwr, dyno_run_index, METRIC_SPEED);
    dyno_graph_ensure_series(g_tq, dyno_run_index, METRIC_SPEED);

    dyno_graph_reset_series(g_speed, dyno_run_index);
    dyno_graph_reset_series(g_rpm, dyno_run_index);
    dyno_graph_reset_series(g_pwr, dyno_run_index);
    dyno_graph_reset_series(g_tq, dyno_run_index);

    if (dyno_samples.n < 2) {
        pthread_mutex_unlock(&dyno_mutex);
        return;
    }

    {
        double t0 = dyno_samples.v[0].t_ms;

        for (unsigned i = 0; i < dyno_samples.n; i++) {
            DynoSample s = dyno_samples.v[i];
            double t_s = (s.t_ms - t0) / 1000.0;

            dyno_graph_add_series_point(g_speed, dyno_run_index, t_s, s.speed_kmh);
            dyno_graph_add_series_point(g_rpm, dyno_run_index, t_s, s.rpm);
        }

        for (unsigned i = 1; i < dyno_samples.n; i++) {
            DynoSample a = dyno_samples.v[i - 1];
            DynoSample b = dyno_samples.v[i];

            double dt = (b.t_ms - a.t_ms) / 1000.0;
            if (dt <= 0.001) continue;

            double v0 = a.speed_kmh / 3.6;
            double v1 = b.speed_kmh / 3.6;
            double acc = (v1 - v0) / dt;

            double mass = gui->params.mass_kg ? gtk_spin_button_get_value(gui->params.mass_kg) : 1200.0;
            double force = mass * acc;
            double power_w = force * v1;

            double rpm = b.rpm;
            double w_rad = (rpm * (2.0 * M_PI)) / 60.0;

            double p_kw = power_w / 1000.0;
            double tq_nm = (1.0 < w_rad) ? (power_w / w_rad) : 0.0;

            if (0 < rpm && isfinite(p_kw)) dyno_graph_add_series_point(g_pwr, dyno_run_index, rpm, p_kw);
            if (0 < rpm && isfinite(tq_nm)) dyno_graph_add_series_point(g_tq, dyno_run_index, rpm, tq_nm);
        }
    }

    g_idle_add(graph_refresh_gsource, g_speed);
    g_idle_add(graph_refresh_gsource, g_rpm);
    g_idle_add(graph_refresh_gsource, g_pwr);
    g_idle_add(graph_refresh_gsource, g_tq);

    pthread_mutex_unlock(&dyno_mutex);
}

static void *dyno_thread_main(void *unused) {
    pthread_mutex_lock(&dyno_mutex);
    dyno_running = true;
    dyno_request_stop = false;
    dyno_samples_clear();
    pthread_mutex_unlock(&dyno_mutex);

    VehicleIFace *iface = config.ephemere.iface;
    if (!iface || dyno_error_feedback_obd(iface)) {
        pthread_mutex_lock(&dyno_mutex);
        dyno_running = false;
        pthread_mutex_unlock(&dyno_mutex);
        return null;
    }

    double start_ms = dyno_now_ms();
    int hz = gui->params.sample_hz ? (int)gtk_spin_button_get_value(gui->params.sample_hz) : 20;
    if (hz < 1) hz = 1;
    double dt_ms = 1000.0 / (double)hz;

    while (true) {
        pthread_mutex_lock(&dyno_mutex);
        gboolean stop = dyno_request_stop;
        pthread_mutex_unlock(&dyno_mutex);

        if (stop) break;
        if (dyno_error_feedback_obd(iface)) break;

        {
            double t_ms = dyno_now_ms();
            double speed = saej1979_data_vehicle_speed(iface, false);
            double rpm = saej1979_data_engine_speed(iface, false);

            if (speed == NAN) speed = 0;
            if (rpm == NAN) rpm = 0;

            pthread_mutex_lock(&dyno_mutex);

            dyno_samples_push(t_ms, speed, rpm);

            {
                Graph *g_speed = dyno_get_graph(AD_GRAPH_TITLE_SPEED);
                Graph *g_rpm   = dyno_get_graph(AD_GRAPH_TITLE_RPM);

                if (g_speed && dyno_samples.n == 1) {
                    GraphSeries *s = dyno_graph_ensure_series(g_speed, dyno_run_index, METRIC_SPEED);
                    if (s) dyno_graph_reset_series(g_speed, dyno_run_index);
                }
                if (g_rpm && dyno_samples.n == 1) {
                    GraphSeries *s = dyno_graph_ensure_series(g_rpm, dyno_run_index, METRIC_ENGINE_SPEED);
                    if (s) dyno_graph_reset_series(g_rpm, dyno_run_index);
                }

                if (g_speed) dyno_graph_add_series_point(g_speed, dyno_run_index, (t_ms - start_ms) / 1000.0, speed);
                if (g_rpm)   dyno_graph_add_series_point(g_rpm, dyno_run_index, (t_ms - start_ms) / 1000.0, rpm);

                pthread_mutex_unlock(&dyno_mutex);

                if (g_speed) g_idle_add(graph_refresh_gsource, g_speed);
                if (g_rpm)   g_idle_add(graph_refresh_gsource, g_rpm);
            }
        }

        {
            struct timespec req;
            req.tv_sec = (time_t)(dt_ms / 1000.0);
            req.tv_nsec = (long)((dt_ms - (double)req.tv_sec * 1000.0) * 1000000.0);
            nanosleep(&req, null);
        }
    }

    pthread_mutex_lock(&dyno_mutex);
    dyno_running = false;
    pthread_mutex_unlock(&dyno_mutex);

    g_idle_add((GSourceFunc)dyno_build_results_from_samples, null);

    return null;
}

static void dyno_start_clicked(GtkButton *b, gpointer unused) {
    pthread_mutex_lock(&dyno_mutex);
    if (dyno_running) {
        pthread_mutex_unlock(&dyno_mutex);
        return;
    }
    dyno_run_index++;
    pthread_mutex_unlock(&dyno_mutex);

    if (!dyno_thread) dyno_thread = malloc(sizeof(*dyno_thread));
    if (!dyno_thread) return;

    if (pthread_create(dyno_thread, null, dyno_thread_main, null) != 0) {
        free(dyno_thread);
        dyno_thread = null;
        return;
    }

    gtk_spinner_start(gui->status.spinner);
}

static void dyno_stop_clicked(GtkButton *b, gpointer unused) {
    pthread_mutex_lock(&dyno_mutex);
    dyno_request_stop = true;
    pthread_mutex_unlock(&dyno_mutex);
    gtk_spinner_stop(gui->status.spinner);
}

static void dyno_reset_clicked(GtkButton *b, gpointer unused) {
    pthread_mutex_lock(&dyno_mutex);

    dyno_request_stop = true;
    dyno_run_index = (unsigned)-1;
    dyno_samples_clear();

    {
        Graph *g_speed = dyno_get_graph(AD_GRAPH_TITLE_SPEED);
        Graph *g_rpm   = dyno_get_graph(AD_GRAPH_TITLE_RPM);
        Graph *g_pwr   = dyno_get_graph(AD_GRAPH_TITLE_POWER);
        Graph *g_tq    = dyno_get_graph(AD_GRAPH_TITLE_TORQUE);

        if (g_speed) dyno_graph_reset(g_speed);
        if (g_rpm) dyno_graph_reset(g_rpm);
        if (g_pwr) dyno_graph_reset(g_pwr);
        if (g_tq) dyno_graph_reset(g_tq);

        pthread_mutex_unlock(&dyno_mutex);

        if (g_speed) g_idle_add(graph_refresh_gsource, g_speed);
        if (g_rpm)   g_idle_add(graph_refresh_gsource, g_rpm);
        if (g_pwr)   g_idle_add(graph_refresh_gsource, g_pwr);
        if (g_tq)    g_idle_add(graph_refresh_gsource, g_tq);
    }
}

static GtkWidget *dyno_make_graph_box(char *title) {
    GtkWidget *frame = gtk_frame_new(null);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);

    GtkWidget *v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(frame), v);

    {
        char title_buf[128];
        graph_display_title(title, title_buf, sizeof(title_buf));
        gtk_box_pack_start(GTK_BOX(v), gtk_label_new(title_buf), false, false, 0);
    }

    GtkWidget *da = gtk_drawing_area_new();
    gtk_widget_set_size_request(da, 320, 240);
    gtk_box_pack_start(GTK_BOX(v), da, true, true, 0);

    Graph *g = graph_new(da, title, (char*)unit_for_metric_title(title));
    if (!g->series) g->series = ad_list_GraphSeries_new();

    if (!dyno_graphs) dyno_graphs = ad_list_Graph_new();
    ad_list_Graph_append(dyno_graphs, g);

    g_signal_connect(G_OBJECT(da), "draw", G_CALLBACK(dyno_graph_on_draw), strdup(title));

    return frame;
}

static GtkWidget *dyno_find_or_make_root_container(GtkWindow *w) {
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(w));
    if (child) return child;
    GtkWidget *v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(w), v);
    gtk_widget_show(v);
    return v;
}

static void dyno_build_gui_widgets() {
    GtkWidget *outer = GTK_WIDGET(gui->container);

    GtkWidget *top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(outer), top, false, false, 0);

    GtkWidget *params = gtk_frame_new("Parameters");
    gtk_box_pack_start(GTK_BOX(top), params, false, false, 0);

    GtkWidget *pg = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(pg), 6);
    gtk_grid_set_column_spacing(GTK_GRID(pg), 8);
    gtk_widget_set_margin_start(pg, 10);
    gtk_widget_set_margin_end(pg, 10);
    gtk_widget_set_margin_top(pg, 10);
    gtk_widget_set_margin_bottom(pg, 10);
    gtk_container_add(GTK_CONTAINER(params), pg);

    GtkAdjustment *adj_mass = gtk_adjustment_new(1200, 200, 6000, 10, 50, 0);
    GtkAdjustment *adj_hz = gtk_adjustment_new(20, 1, 100, 1, 5, 0);

    gui->params.mass_kg = GTK_SPIN_BUTTON(gtk_spin_button_new(adj_mass, 1, 0));
    gui->params.sample_hz = GTK_SPIN_BUTTON(gtk_spin_button_new(adj_hz, 1, 0));

    gtk_grid_attach(GTK_GRID(pg), gtk_label_new("Mass (kg)"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(pg), GTK_WIDGET(gui->params.mass_kg), 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(pg), gtk_label_new("Sample rate (Hz)"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(pg), GTK_WIDGET(gui->params.sample_hz), 1, 1, 1, 1);

    GtkWidget *actions = gtk_frame_new("Actions");
    gtk_box_pack_start(GTK_BOX(top), actions, true, true, 0);

    GtkWidget *ab = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(ab, 10);
    gtk_widget_set_margin_end(ab, 10);
    gtk_widget_set_margin_top(ab, 10);
    gtk_widget_set_margin_bottom(ab, 10);
    gtk_container_add(GTK_CONTAINER(actions), ab);

    gui->actions.btn_start = GTK_BUTTON(gtk_button_new_with_label("Start"));
    gui->actions.btn_stop  = GTK_BUTTON(gtk_button_new_with_label("Stop"));
    gui->actions.btn_reset = GTK_BUTTON(gtk_button_new_with_label("Reset"));

    gtk_box_pack_start(GTK_BOX(ab), GTK_WIDGET(gui->actions.btn_start), false, false, 0);
    gtk_box_pack_start(GTK_BOX(ab), GTK_WIDGET(gui->actions.btn_stop), false, false, 0);
    gtk_box_pack_start(GTK_BOX(ab), GTK_WIDGET(gui->actions.btn_reset), false, false, 0);

    gui->status.spinner = GTK_SPINNER(gtk_spinner_new());
    gtk_box_pack_end(GTK_BOX(ab), GTK_WIDGET(gui->status.spinner), false, false, 0);

    GtkWidget *status = gtk_frame_new("Live");
    gtk_box_pack_start(GTK_BOX(outer), status, false, false, 0);

    GtkWidget *sg = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(sg), 6);
    gtk_grid_set_column_spacing(GTK_GRID(sg), 10);
    gtk_widget_set_margin_start(sg, 10);
    gtk_widget_set_margin_end(sg, 10);
    gtk_widget_set_margin_top(sg, 10);
    gtk_widget_set_margin_bottom(sg, 10);
    gtk_container_add(GTK_CONTAINER(status), sg);

    gui->status.lbl_state = GTK_LABEL(gtk_label_new("Idle"));
    gui->status.lbl_time  = GTK_LABEL(gtk_label_new("0.00 s"));
    gui->status.lbl_speed = GTK_LABEL(gtk_label_new("0.0 km/h"));
    gui->status.lbl_rpm   = GTK_LABEL(gtk_label_new("0 r/min"));
    gui->status.lbl_pwr   = GTK_LABEL(gtk_label_new("0.00 kW"));
    gui->status.lbl_hp    = GTK_LABEL(gtk_label_new("0.00 hp"));
    gui->status.lbl_tq    = GTK_LABEL(gtk_label_new("0.0 N·m"));

    gtk_grid_attach(GTK_GRID(sg), gtk_label_new("Power"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(sg), GTK_WIDGET(gui->status.lbl_pwr), 1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(sg), gtk_label_new("Horse Power"), 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(sg), GTK_WIDGET(gui->status.lbl_hp), 3, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(sg), gtk_label_new("Torque"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(sg), GTK_WIDGET(gui->status.lbl_tq), 1, 3, 1, 1);

    gtk_grid_attach(GTK_GRID(sg), gtk_label_new("State"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(sg), GTK_WIDGET(gui->status.lbl_state), 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(sg), gtk_label_new("Time"), 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(sg), GTK_WIDGET(gui->status.lbl_time), 3, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(sg), gtk_label_new("Speed"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(sg), GTK_WIDGET(gui->status.lbl_speed), 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(sg), gtk_label_new("RPM"), 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(sg), GTK_WIDGET(gui->status.lbl_rpm), 3, 1, 1, 1);

    GtkWidget *peak = gtk_frame_new("Peak");
    gtk_box_pack_start(GTK_BOX(outer), peak, false, false, 0);

    GtkWidget *peak_metrics = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(peak_metrics), 6);
    gtk_grid_set_column_spacing(GTK_GRID(peak_metrics), 10);
    gtk_widget_set_margin_start(peak_metrics, 10);
    gtk_widget_set_margin_end(peak_metrics, 10);
    gtk_widget_set_margin_top(peak_metrics, 10);
    gtk_widget_set_margin_bottom(peak_metrics, 10);
    gtk_container_add(GTK_CONTAINER(peak), peak_metrics);

    gui->peak.lbl_time  = GTK_LABEL(gtk_label_new("0.00 s"));
    gui->peak.lbl_speed = GTK_LABEL(gtk_label_new("0.0 km/h"));
    gui->peak.lbl_rpm   = GTK_LABEL(gtk_label_new("0 r/min"));
    gui->peak.lbl_pwr   = GTK_LABEL(gtk_label_new("0.00 kW"));
    gui->peak.lbl_hp    = GTK_LABEL(gtk_label_new("0.00 hp"));
    gui->peak.lbl_tq    = GTK_LABEL(gtk_label_new("0.0 N·m"));

    gtk_grid_attach(GTK_GRID(peak_metrics), gtk_label_new("Time"), 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(peak_metrics), GTK_WIDGET(gui->peak.lbl_time), 3, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(peak_metrics), gtk_label_new("Speed"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(peak_metrics), GTK_WIDGET(gui->peak.lbl_speed), 1, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(peak_metrics), gtk_label_new("RPM"), 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(peak_metrics), GTK_WIDGET(gui->peak.lbl_rpm), 3, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(peak_metrics), gtk_label_new("Power"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(peak_metrics), GTK_WIDGET(gui->peak.lbl_pwr), 1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(peak_metrics), gtk_label_new("Horse Power"), 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(peak_metrics), GTK_WIDGET(gui->peak.lbl_hp), 3, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(peak_metrics), gtk_label_new("Torque"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(peak_metrics), GTK_WIDGET(gui->peak.lbl_tq), 1, 3, 1, 1);

    GtkWidget *graphs_frame = gtk_frame_new("Graphs");
    gtk_box_pack_start(GTK_BOX(outer), graphs_frame, true, true, 0);

    GtkWidget *gg = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(gg), 10);
    gtk_grid_set_column_spacing(GTK_GRID(gg), 10);
    gtk_widget_set_margin_start(gg, 10);
    gtk_widget_set_margin_end(gg, 10);
    gtk_widget_set_margin_top(gg, 10);
    gtk_widget_set_margin_bottom(gg, 10);
    gtk_container_add(GTK_CONTAINER(graphs_frame), gg);

    GtkWidget *g1 = dyno_make_graph_box(AD_GRAPH_TITLE_SPEED);
    GtkWidget *g2 = dyno_make_graph_box(AD_GRAPH_TITLE_RPM);
    GtkWidget *g3 = dyno_make_graph_box(AD_GRAPH_TITLE_POWER);
    GtkWidget *g4 = dyno_make_graph_box(AD_GRAPH_TITLE_TORQUE);

    gtk_grid_attach(GTK_GRID(gg), g1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(gg), g2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(gg), g3, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(gg), g4, 1, 1, 1, 1);

    g_signal_connect(G_OBJECT(gui->actions.btn_start), "clicked", G_CALLBACK(dyno_start_clicked), null);
    g_signal_connect(G_OBJECT(gui->actions.btn_stop),  "clicked", G_CALLBACK(dyno_stop_clicked), null);
    g_signal_connect(G_OBJECT(gui->actions.btn_reset), "clicked", G_CALLBACK(dyno_reset_clicked), null);

    g_timeout_add(200, dyno_ui_tick, null);

    gtk_widget_show_all(outer);
}
*/
static void init(final GtkBuilder *builder) {
    if (gui != null) {
        log_msg(LOG_WARNING, "Already init dyno mod");
        return;
    }

    DynoGui g = {
        .window = GTK_WINDOW(gtk_builder_get_object(builder, "dyno-window")),
        .container = GTK_BOX(gtk_builder_get_object(builder, "dyno-widgets-container")),
        .menuBar = {
            .data = {
                .all = GTK_MENU_ITEM(gtk_builder_get_object(builder, "dyno-menubar-data-all")),
                .filter_by = GTK_WIDGET(gtk_builder_get_object(builder, "dyno-menubar-data-filter-by"))
            }
        },
        .errorFeedback = ERROR_FEEDBACK_RETRIEVE_WINDOWS(builder)
    };

    gui = (DynoGui*)malloc(sizeof(DynoGui));
    (*gui) = g;

    assert(0 != g_signal_connect(G_OBJECT(gui->window), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), null));

    error_feedback_windows_init(gui->errorFeedback);

    //MENUBAR_DATA_CONNECT()

    //dyno_build_gui_widgets();
}

static void end() {
    pthread_mutex_lock(&dyno_mutex);
    dyno_request_stop = true;
    pthread_mutex_unlock(&dyno_mutex);

    if (dyno_thread) {
        pthread_join(*dyno_thread, null);
        free(dyno_thread);
        dyno_thread = null;
    }

    pthread_mutex_lock(&dyno_mutex);
    //dyno_samples_clear();
    pthread_mutex_unlock(&dyno_mutex);

    if (dyno_graphs) {
        ad_list_Graph_free(dyno_graphs);
        dyno_graphs = null;
    }

    if (gui != null) {
        free(gui);
        gui = null;
    }
}

static void show() {
    gtk_window_show_ensure_ontop(GTK_WIDGET(gui->window));
}

static void hide() {
    gtk_widget_hide_on_main_thread(GTK_WIDGET(gui->window));
}

mod_gui *mod_gui_dyno_new() {
    mod_gui *mg = (mod_gui*)malloc(sizeof(mod_gui));
    mg->init = init;
    mg->end = end;
    mg->name = strdup("Dynamometer");
    mg->show = show;
    mg->hide = hide;
    return mg;
}