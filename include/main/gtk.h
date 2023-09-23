#ifndef __CUSTOM_GTK_H
#define __CUSTOM_GTK_H

#include <gtk/gtk.h>
#include "globals.h"

void gtk_widget_hide_on_main_thread(GtkWidget *widget);
void gtk_widget_show_on_main_thread(GtkWidget *widget);
gboolean gtk_widget_generic_onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused);

#endif
