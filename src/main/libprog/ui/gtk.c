#include "libprog/ui/gtk.h"

gboolean gtk_combo_box_text_prevent_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data) {
    gtk_propagate_event(gtk_widget_get_parent(widget), (GdkEvent*)event);
    return TRUE;
}

gboolean gtk_widget_hide_on_main_thread_wrapper(gpointer data) {
    gtk_widget_hide((GtkWidget*)data);
    return false;
}
gboolean gtk_widget_show_on_main_thread_wrapper(gpointer data) {
    gtk_widget_show((GtkWidget*)data);
    return false;
}
void gtk_widget_hide_on_main_thread(GtkWidget *widget) {
    gdk_threads_add_idle(G_SOURCE_FUNC(&gtk_widget_hide_on_main_thread_wrapper),widget);
}
void gtk_widget_show_on_main_thread(GtkWidget *widget) {
    gdk_threads_add_idle(G_SOURCE_FUNC(&gtk_widget_show_on_main_thread_wrapper),widget);
}
gboolean gtk_widget_generic_onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused) {
    gtk_widget_hide(dialog);
    return true;
}

GTK_GENERATE_PROGRESS_BAR_FILL(int,gtk_progress_bar_fill_from_int)
GTK_GENERATE_PROGRESS_BAR_FILL(double,gtk_progress_bar_fill_from_double)
GTK_GENERATE_PROGRESS_BAR_FILL(int,gtk_progress_bar_fill)

bool gtk_widget_printf(GtkWidget *widget, const char *format, ...) {
    va_list ap;
    bool result;
    va_start(ap, format);

    char *txt;
    if (compat_vasprintf(&txt, format, ap) == -1) {
        log_msg(LOG_ERROR, "Fill widget impossible");
        result = false;
    } else {
        if (GTK_IS_LABEL(widget)) {
            gtk_label_set_text(GTK_LABEL(widget), txt);
            result = true;
        } else if (GTK_IS_ENTRY(widget)) {
            gtk_entry_set_text(GTK_ENTRY(widget), txt);
            result = true;
        } else {
            log_msg(LOG_ERROR, "Unsupported widget type");
            result = false;
        }
        free(txt);
    }

    va_end(ap);
    return result;
}
static gboolean present_window_cleaner(gpointer window) {
    final int ms = 1000;
    usleep(20 * ms);
    gtk_window_set_keep_above(GTK_WINDOW(window), false);
    return false;
}
static gboolean present_window(gpointer data) {
    GtkWidget * window = GTK_WIDGET(data);
    gtk_widget_show_now(GTK_WIDGET(window));
    gtk_window_set_keep_above(GTK_WINDOW(window), true);
    g_idle_add(present_window_cleaner, (gpointer)window);
    gtk_window_present(GTK_WINDOW(window));
    return false;
}
void gtk_window_show_ensure_ontop(GtkWidget * window) {
    gdk_threads_add_idle(G_SOURCE_FUNC(&present_window),window);
}
