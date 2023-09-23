#include "gtk.h"

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
