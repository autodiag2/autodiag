#include "ui/documentation.h"

DocumentationGui *documentationgui = null;

gboolean documentation_onclose(final GtkWidget *dialog, final GdkEvent *event, gpointer unused) {
    gtk_widget_hide(documentationgui->window);
    return true;
}

void documentation_show_window() {
   gtk_widget_show_now(documentationgui->window);
}

void module_init_documentation(GtkBuilder *builder) {
    if ( documentationgui == null ) {
        documentationgui = (DocumentationGui *)malloc(sizeof(DocumentationGui));
        DocumentationGui g = {
            .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-documentation"))
        };
        *documentationgui = g;
        g_signal_connect(G_OBJECT(documentationgui->window),"delete-event",G_CALLBACK(documentation_onclose),NULL);
        gtk_builder_add_callback_symbol(builder,"show-window-documentation",&documentation_show_window);
    }
}
