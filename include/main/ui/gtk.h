#ifndef __CUSTOM_GTK_H
#define __CUSTOM_GTK_H

#define _GNU_SOURCE
#include <stdio.h>
#include <gtk/gtk.h>
#include "libautodiag/lib.h"

/**
 * Any struct that inherit this is a threaded action, an action that runs on a thread.
 * eg a click on a button
 */
typedef struct {
    /**
     * thread onto which the read operation runs
     */
    pthread_t *thread;
} ThreadedAction;

void gtk_widget_hide_on_main_thread(GtkWidget *widget);
void gtk_widget_show_on_main_thread(GtkWidget *widget);
gboolean gtk_widget_generic_onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused);

#define GTK_GENERATE_HEADER_PROGRESS_BAR_FILL(type,sym) void sym(GtkProgressBar *bar, type value, type min, type max, type error, char *format)
#define GTK_GENERATE_PROGRESS_BAR_FILL(type,sym) GTK_GENERATE_HEADER_PROGRESS_BAR_FILL(type,sym) { \
    char * value_txt;\
    if ( asprintf(&value_txt, format, value) == -1 ) {\
        log_msg(LOG_ERROR, "Fill progress bar impossible");\
    } else {\
        if ( strcmp(value_txt,gtk_progress_bar_get_text(bar)) == 0 ) {\
                \
        } else {\
          if ( value == error ) {\
          \
          } else { \
            final double value_percent = compute_normalized_of(value,min,max);\
            if ( log_get_level() == LOG_DEBUG ) {\
                if ( value_percent < 0 || 1 < value_percent ) { \
                    log_msg(LOG_DEBUG, "out of range value detected value_percent=%.2f value=%d min=%d max=%d\n", value_percent, (int)value, (int)min, (int)max);\
                }\
            }\
            gtk_progress_bar_set_text(bar, value_txt);\
            gtk_progress_bar_set_fraction(bar, value_percent);\
          }\
        }\
        free(value_txt);\
    } \
}

#define GTK_PROGRESS_BAR_FILL_GSOURCE_SYM(sym,type,min,max,error,format,bar) gboolean sym(gpointer data) { \
    type *value = data; \
    gtk_progress_bar_fill_from_##type(bar, \
        *value, \
        min,max,error, \
        format \
    ); \
    free(data); \
    return false; \
}

GTK_GENERATE_HEADER_PROGRESS_BAR_FILL(int,gtk_progress_bar_fill_from_int);
GTK_GENERATE_HEADER_PROGRESS_BAR_FILL(double,gtk_progress_bar_fill_from_double);
GTK_GENERATE_HEADER_PROGRESS_BAR_FILL(int,gtk_progress_bar_fill);
bool gtk_label_printf(GtkLabel *label, const char *format, ...);

static int get_container_child_count(GtkContainer *container) {
    GList *children = gtk_container_get_children(container);
    int count = g_list_length(children);
    g_list_free(children);
    return count;
}

#endif
