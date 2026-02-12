#include "ui/ui_mod.h"

int mod_gui_cmp(mod_gui * m1, mod_gui *m2) {
    return m1 - m2;
}
AD_LIST_SRC(mod_gui)

void list_mod_gui_build(list_mod_gui * mods, final GtkBuilder * builder) {
    for(int i = 0; i < mods->size; i++) {
        mods->list[i]->init(builder);
    }
}