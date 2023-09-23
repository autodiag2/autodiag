#ifndef __DOCUMENTATION_H
#define __DOCUMENTATION_H

#include <gtk/gtk.h>
#include "globals.h"

typedef struct {
    GtkWidget *window;
} DocumentationGui;

extern DocumentationGui *documentationgui;

void documentation_show_window();
void module_init_documentation(GtkBuilder *builder);

#endif
