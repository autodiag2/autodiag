#ifndef __DOCUMENTATION_H
#define __DOCUMENTATION_H

#include "ui.h"
#include "ui_mod.h"

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

mod_gui * mod_gui_documentation_new();

#endif
