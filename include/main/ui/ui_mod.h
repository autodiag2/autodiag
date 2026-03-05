#ifndef __UI_MOD_H
#define __UI_MOD_H

#include "ui/ui.h"

typedef struct {
    char *name;
    void (*init)(final GtkBuilder *builder);
    void (*end)();
    void (*show)();
    void (*hide)();
} mod_gui;

AD_LIST_H(mod_gui)
void ad_list_mod_gui_build(ad_list_mod_gui * mods, final GtkBuilder * builder);

#endif