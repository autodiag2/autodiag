#include "ui/documentation.h"

DocumentationGui *documentationgui = null;

void documentation_set_manuals_directory() {
    final char * basepath = installation_folder_resolve("data" PATH_FOLDER_DELIM "manuals");
    gtk_file_chooser_set_current_folder(documentationgui->fileChooser,basepath);
    free(basepath);
}

void documentation_switch_tabs (GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data) {
    documentation_set_manuals_directory();
}

void documentation_show_window() {
    documentation_set_manuals_directory();
    gtk_widget_show_now(GTK_WIDGET(documentationgui->window));
}

void documentation_open_manuals() {
    GSList *list = gtk_file_chooser_get_uris(documentationgui->fileChooser);
    for(GSList *ptr = list; ptr != null; ptr = ptr->next) {
        GError *error;
        gtk_show_uri_on_window (documentationgui->window,
                        (gchar*)ptr->data,
                        GDK_CURRENT_TIME,
                        &error);
        log_msg(LOG_INFO, "openning manuals at : %s", (gchar*)ptr->data);
    }
}

void module_init_documentation(GtkBuilder *builder) {
    if ( documentationgui == null ) {
        documentationgui = (DocumentationGui *)malloc(sizeof(DocumentationGui));
        DocumentationGui g = {
            .window = GTK_WINDOW(gtk_builder_get_object(builder, "window-documentation")),
            .tabber = (GtkNotebook*)(gtk_builder_get_object(builder, "window-documentation-tabber")),
            .fileChooser = GTK_FILE_CHOOSER (gtk_builder_get_object(builder, "window-documentation-file-chooser")),
            .about = {
                .buildTime = GTK_LABEL (gtk_builder_get_object(builder, "window-documentation-about-app-build-time")),
                .version = GTK_LABEL (gtk_builder_get_object(builder, "window-documentation-about-app-version")),                
                .name = GTK_LABEL (gtk_builder_get_object(builder, "window-documentation-about-app-name")),                
                .maintainer = GTK_LABEL (gtk_builder_get_object(builder, "window-documentation-about-app-maintainer")),                
                .description = GTK_TEXT_VIEW (gtk_builder_get_object(builder, "window-documentation-about-app-desc")),                
                .descriptionText = GTK_TEXT_BUFFER (gtk_builder_get_object(builder, "window-documentation-description-buffer"))
            }
        };
        *documentationgui = g;
        gtk_label_set_text(g.about.buildTime,__DATE__ " " __TIME__);
        gtk_label_set_text(g.about.name,APP_NAME);
        gtk_label_set_text(g.about.version,APP_VERSION);
        gtk_label_set_text(g.about.maintainer,APP_MAINTAINER);
        gtk_text_buffer_set_text (g.about.descriptionText, APP_DESC, strlen(APP_DESC));
        g_signal_connect(G_OBJECT(g.tabber),"switch-page",G_CALLBACK(documentation_switch_tabs),NULL);
        g_signal_connect(G_OBJECT(g.window),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),NULL);
        g_signal_connect(G_OBJECT(g.fileChooser),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),NULL);        
        gtk_builder_add_callback_symbol(builder,"show-window-documentation",&documentation_show_window);
        gtk_builder_add_callback_symbol(builder,"documentation-open-manuals",&documentation_open_manuals);
    }
}
