#ifndef __DOCUMENTATION_H
#define __DOCUMENTATION_H

#include "ui.h"

typedef struct {
    GtkWindow *window;
    GtkFileChooser *fileChooser;
    GtkNotebook* tabber;
    struct {
        GtkLabel* buildTime;
        GtkLabel* version;
        GtkLabel* name;
        GtkLabel* maintainer;
        GtkTextView* description;
        GtkTextBuffer* descriptionText;
    } about;
} DocumentationGui;

extern DocumentationGui *documentationgui;

void documentation_show_window();
void module_init_documentation(GtkBuilder *builder);

#endif
