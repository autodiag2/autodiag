#include "ui/documentation.h"

static DocumentationGui *gui = null;

static void set_manuals_directory() {
    final char * basepath = installation_folder_resolve("data" PATH_FOLDER_DELIM "manuals");
    gtk_file_chooser_set_current_folder(gui->fileChooser,basepath);
    free(basepath);
}

static void switch_tabs(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data) {
    set_manuals_directory();
}

static void show() {
    set_manuals_directory();
    gtk_window_show_ensure_ontop(GTK_WIDGET(gui->window));
}

static void open_manuals() {
    GSList *list = gtk_file_chooser_get_uris(gui->fileChooser);
    for(GSList *ptr = list; ptr != null; ptr = ptr->next) {
        GError *error;
        gtk_show_uri_on_window (gui->window,
                        (gchar*)ptr->data,
                        GDK_CURRENT_TIME,
                        &error);
        log_msg(LOG_INFO, "openning manuals at : %s", (gchar*)ptr->data);
    }
}

void module_init_documentation(GtkBuilder *builder) {
    if ( gui == null ) {
        gui = (DocumentationGui *)malloc(sizeof(DocumentationGui));
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
        *gui = g;
        gtk_label_set_text(g.about.buildTime,__DATE__ " " __TIME__);
        gtk_label_set_text(g.about.name,APP_NAME);
        gtk_label_set_text(g.about.version,APP_VERSION);
        gtk_label_set_text(g.about.maintainer,APP_MAINTAINER);
        gtk_text_buffer_set_text (g.about.descriptionText, APP_DESC, strlen(APP_DESC));
        g_signal_connect(G_OBJECT(g.tabber),"switch-page",G_CALLBACK(switch_tabs),NULL);
        g_signal_connect(G_OBJECT(g.window),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),NULL);
        g_signal_connect(G_OBJECT(g.fileChooser),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),NULL);        
        gtk_builder_add_callback_symbol(builder,"show-window-documentation",&show);
        gtk_builder_add_callback_symbol(builder,"documentation-open-manuals",&open_manuals);
    }
}
