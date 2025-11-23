#include "ui/lib/gtk.h"

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

bool gtk_label_printf(GtkLabel *label, const char *format, ...) {
    va_list ap;
    final bool result;
    va_start(ap, format);

    char *txt;
    if ( vasprintf(&txt, format, ap) == -1 ) {
        log_msg(LOG_ERROR, "Fill label impossible");
        result = false;
    } else {
        gtk_label_set_text(label, txt);
        free(txt);
        result = true;
    }
    va_end(ap);
    return result;
}
