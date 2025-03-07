#include "ui/trouble_code_reader.h"

TroubleCodeReaderGui * tcgui = null;
SAEJ1979_DTC_list * dtc_list = null;

void trouble_code_reader_clear_dtc_description() {
    gtk_text_buffer_set_text(tcgui->dtc.explanationText, "", 0);
    gtk_text_buffer_set_text(tcgui->dtc.descriptionText, "", 0);
    gtk_text_buffer_set_text(tcgui->dtc.causeSolutionText, "", 0);
}

void read_codes_hide_window() {
    gtk_widget_hide_on_main_thread(tcgui->window);
}

void read_codes_show_window() {
    gtk_widget_show_on_main_thread(tcgui->window);
}

void read_codes_show_clear_confirm() {
    gtk_widget_show_on_main_thread(tcgui->clear.confirm);
}

void read_codes_go_back_main_menu() {
    module_debug(MODULE_CODES_READER, "Go back main menu");
    read_codes_hide_window();
}

static void trouble_code_reader_button_click_clean_up_routine(void *arg) {
    obd_thread_cleanup_routine(arg);
    gtk_spinner_stop(tcgui->actionWaitIcon);
}

void trouble_code_reader_read_codes_error_popup_ok() {
    gtk_widget_hide_on_main_thread(tcgui->errorFeedback.obd);
}
void trouble_code_reader_read_codes_error_serial_ok() {
    gtk_widget_hide_on_main_thread(GTK_WIDGET(tcgui->errorFeedback.serial));
}
void trouble_code_reader_read_codes_no_obd_data_ok() {
    gtk_widget_hide_on_main_thread(tcgui->noObdData);
}
gboolean trouble_code_reader_read_codes_show_ecus_buffer_gsource(gpointer data) {
    char *msg = (char*)data; 
    gtk_text_buffer_set_text(tcgui->menuBar.showECUsBuffer.text, msg, strlen(msg));
    free(msg);
    return false;
}
void trouble_code_reader_read_codes_show_ecus_buffer_internal() {
    final OBDIFace* iface = config.ephemere.iface;
    char * result = strdup("");
    if ( iface != null ) {
        Vehicle * vehicle = iface->vehicle;    
        for(int i = 0; i < vehicle->ecus_len; i++) {
            ECU* ecu = vehicle->ecus[i];
            {
                char *tmp;
                char * ascii_dump = buffer_bin_to_ascii_hex(ecu->address);
                asprintf(&tmp,"%sOn ECU '%s' at address: %s\n",result,ecu->name == null ? "" : ecu->name,ascii_dump);
                free(result);
                result = tmp;
                free(ascii_dump);
            }
            
            LIST_FOREACH(ecu->obd_data_buffer,Buffer,buffer,
                char * dump = buffer_to_hexdump(buffer->buffer, buffer->size);
                char *tmp;
                asprintf(&tmp, "%sdata#%d:\n%s",result,list_element_index+1,dump);
                free(result);
                result = tmp;
                free(dump);
            )
        }
    }
    g_idle_add(trouble_code_reader_read_codes_show_ecus_buffer_gsource, (gpointer)result);
}

THREAD_WRITE_DAEMON(
        trouble_code_reader_read_codes_show_ecus_buffer_daemon,
        trouble_code_reader_read_codes_show_ecus_buffer_internal,
        trouble_code_reader_button_click_clean_up_routine, tcgui->menuBar.showECUsBuffer.thread
)

void trouble_code_reader_read_codes_show_ecus_buffer() {
    gtk_widget_show_on_main_thread(tcgui->menuBar.showECUsBuffer.window);
    thread_allocate_and_start(&tcgui->menuBar.showECUsBuffer.thread,&trouble_code_reader_read_codes_show_ecus_buffer_daemon);
}

gboolean trouble_code_reader_read_codes_set_mil_gsource(gpointer data) {
    bool mil = *((bool*)data);
    gtk_label_set_text(tcgui->mil.text, mil ? "MIL is ON":"MIL is OFF");
    gtk_widget_set_visible((GtkWidget*)tcgui->mil.iconOn,mil);
    gtk_widget_set_visible((GtkWidget*)tcgui->mil.iconOff,!mil);
    free(data);
    return false;
}

gboolean trouble_code_reader_read_codes_set_dtc_count_gsource(gpointer data) {
    char *dtcStr = (char*)data;
    gtk_label_set_text(tcgui->dtc.count, dtcStr);
    free(dtcStr);
    return false;
}
gboolean trouble_code_reader_read_codes_set_dtc_list_gsource(gpointer data) {
    SAEJ1979_DTC_list *dtc_list = (SAEJ1979_DTC_list *)data;
    GList *ptr = gtk_container_get_children((GtkContainer*)tcgui->dtc.list);
    while(ptr != null) {
        final GList *ptr_next = ptr->next;
        gtk_widget_destroy(ptr->data);
        ptr = ptr_next;
    }
    if ( dtc_list != null ) {
        LIST_FOREACH(dtc_list,SAEJ1979_DTC,dtc,
            char * dtc_string = saej1979_dtc_to_string(dtc);
            GtkWidget *label = gtk_label_new(dtc_string);
            gtk_container_add((GtkContainer*)tcgui->dtc.list,label);
            gtk_widget_show(label);
            free(dtc_string);
        )
    }
    return false;
}

void trouble_code_reader_read_codes_daemon_internal() {
    final OBDIFace* iface = config.ephemere.iface;
    if ( ! trouble_code_reader_error_feedback_obd(iface) ) {
        gtk_spinner_start(tcgui->actionWaitIcon);
        // The state of the port may change if thereis an error during the retrieve
        final bool mil = saej1979_data_mil_status(iface,false);
        if ( ! trouble_code_reader_error_feedback_obd(iface) ) {
            {
                bool * mil_ptr = (bool*)malloc(sizeof(bool));
                *mil_ptr = mil;
                g_idle_add(trouble_code_reader_read_codes_set_mil_gsource, (gpointer)mil_ptr);
            }
            {
                if ( dtc_list != null ) {
                    SAEJ1979_DTC_list_free(dtc_list);
                    dtc_list = null;
                }
                final SAEJ1979_DTC_list * pending = SAEJ1979_DTC_list_new();
                if ( gtk_toggle_button_get_active(tcgui->read.stored) ) {
                    SAEJ1979_DTC_list_append_list(pending,saej1979_retrieve_stored_dtcs(iface));
                }
                if ( gtk_toggle_button_get_active(tcgui->read.pending) ) {
                    SAEJ1979_DTC_list_append_list(pending,saej1979_retrieve_pending_dtcs(iface));
                }
                if ( gtk_toggle_button_get_active(tcgui->read.permanent) ) {
                    SAEJ1979_DTC_list_append_list(pending,saej1979_retrieve_permanent_dtcs(iface));
                }
                if ( 0 < pending->size ) {
                    dtc_list = pending;
                }
                g_idle_add(trouble_code_reader_read_codes_set_dtc_list_gsource, (gpointer)dtc_list);
            }
            {
                int dtcCount = 0;
                if ( dtc_list != null ) {
                    dtcCount = dtc_list->size;
                }
                char *dtcStr;
                asprintf(&dtcStr,"%d DTC%s", dtcCount, 1 < dtcCount ? "s" : "" );
                g_idle_add(trouble_code_reader_read_codes_set_dtc_count_gsource, (gpointer)dtcStr);
            }
        }
    }
    trouble_code_reader_clear_dtc_description();
}

THREAD_WRITE_DAEMON(
        trouble_code_reader_read_codes_daemon, trouble_code_reader_read_codes_daemon_internal,
        trouble_code_reader_button_click_clean_up_routine, tcgui->read.thread
)

void trouble_code_reader_read_codes() {
    thread_allocate_and_start(&tcgui->read.thread, &trouble_code_reader_read_codes_daemon);
}

void trouble_code_reader_clear_codes_daemon_internal() {
    gtk_spinner_start(tcgui->actionWaitIcon);
    final OBDIFace* iface = config.ephemere.iface;
    if ( ! trouble_code_reader_error_feedback_obd(iface) ) {
        saej1979_clear_dtc_and_stored_values(iface);
    }
    gtk_widget_hide_on_main_thread(tcgui->clear.confirm);
    trouble_code_reader_clear_dtc_description();
    trouble_code_reader_read_codes_daemon_internal();
}
THREAD_WRITE_DAEMON(
        trouble_code_reader_clear_codes_daemon, trouble_code_reader_clear_codes_daemon_internal,
        trouble_code_reader_button_click_clean_up_routine, tcgui->clear.thread
)

gboolean read_codes_onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused) {
    log_msg(LOG_DEBUG, "Close event received");
    THREAD_CANCEL(tcgui->read.thread);
    THREAD_CANCEL(tcgui->clear.thread);
    THREAD_CANCEL(tcgui->menuBar.showECUsBuffer.thread);
    return gtk_widget_generic_onclose(dialog,event,unused);
}

void trouble_code_reader_confirm_cancel() {
    gtk_widget_hide(tcgui->clear.confirm);
}
void trouble_code_reader_confirm_confirm() {
    thread_allocate_and_start(&tcgui->clear.thread,&trouble_code_reader_clear_codes_daemon);
}

void trouble_code_reader_append_multi_brand_explanation(GtkTextBuffer *buffer, SAEJ1979_DTC_DESCRIPTION *desc, char *part) {
    if ( part == null ) {

    } else {
        char *textEngine;
        if ( desc->car->ecu == null || desc->car->ecu->model == null ) {
            textEngine = strdup("");
        } else {
            asprintf(&textEngine, "(ecu: %s)", desc->car->ecu == null ? "" : desc->car->ecu->model == null ? "" : desc->car->ecu->model);
        }
        char *text;
        asprintf(&text, " %s %s %s\n\t%s\n", desc->car->brand, desc->car->model == null ? "" : desc->car->model, textEngine, part);
        free(textEngine);
        GtkTextIter iter;
        gtk_text_buffer_get_iter_at_offset(buffer,&iter,-1);
        gtk_text_buffer_insert(buffer, &iter,text, -1);
        free(text);
    }
}

void trouble_code_reader_dtc_selected(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    trouble_code_reader_clear_dtc_description();
    if ( row != null ) {
        const char * selectedDTC = gtk_label_get_text((GtkLabel*)gtk_bin_get_child((GtkBin*)row));
        SAEJ1979_DTC *dtc = SAEJ1979_DTC_list_get(dtc_list,(char*)selectedDTC);

        for(int i = 0; i < dtc->description->size; i++) {
            SAEJ1979_DTC_DESCRIPTION desc = dtc->description->list[i];
            trouble_code_reader_append_multi_brand_explanation(tcgui->dtc.causeSolutionText,&desc,desc.solution);
            trouble_code_reader_append_multi_brand_explanation(tcgui->dtc.descriptionText,&desc,desc.reason);
        }
        final char * explanation = saej1979_dtc_categorization_string(dtc);
        gtk_text_buffer_set_text(tcgui->dtc.explanationText,explanation,strlen(explanation));
        free(explanation);
    }
}

void module_init_read_codes(GtkBuilder *builder) {
    if ( tcgui == null ) {
        module_debug_init(MODULE_CODES_READER);
        tcgui = (TroubleCodeReaderGui*)malloc(sizeof(TroubleCodeReaderGui));
        TroubleCodeReaderGui g = {
            .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-read-codes")),
            .noObdData = GTK_WIDGET (gtk_builder_get_object (builder, "window-no-obd-data")),
            .errorFeedback = ERROR_FEEDBACK_RETRIEVE_WINDOWS(builder),
            .menuBar = {
                .showECUsBuffer = {
                    .window = GTK_WIDGET (gtk_builder_get_object (builder, "window-show-ecus-buffer")),
                    .textView = (GtkTextView*) (gtk_builder_get_object (builder, "window-show-ecus-buffer-text")),
                    .text = gtk_text_buffer_new(null),
                    .thread = null
                }
            },
            .mil = {
                .text = (GtkLabel*)gtk_builder_get_object (builder, "window-read-codes-mil-text"),
                .iconOn = (GtkImage*)gtk_builder_get_object (builder,"window-read-codes-mil-icon-on"),
                .iconOff = (GtkImage*)gtk_builder_get_object (builder,"window-read-codes-mil-icon-off")
            },
            .dtc = {
                .count = (GtkLabel*)gtk_builder_get_object (builder, "window-read-codes-dtc-count-text"),
                .list = (GtkListBox*)gtk_builder_get_object (builder, "window-read-codes-dtc-list"),
                .description = (GtkTextView*)gtk_builder_get_object (builder, "window-read-codes-dtc-description"),
                .descriptionText = gtk_text_buffer_new(null),
                .causeSolution = (GtkTextView*)gtk_builder_get_object (builder, "window-read-codes-dtc-cause-solution-description"),
                .causeSolutionText = gtk_text_buffer_new(null),
                .explanation = (GtkTextView*)gtk_builder_get_object (builder, "window-read-codes-dtc-cause-solution-explanation"),
                .explanationText = gtk_text_buffer_new(null),
            },
            .read = {
                .stored = (GtkToggleButton*)gtk_builder_get_object (builder, "window-read-codes-read-stored"),
                .pending = (GtkToggleButton*)gtk_builder_get_object (builder, "window-read-codes-read-pending"),
                .permanent = (GtkToggleButton*)gtk_builder_get_object (builder, "window-read-codes-read-permanent"),
                .thread = null
            },
            .clear = {
                .confirm = GTK_WIDGET (gtk_builder_get_object (builder, "window-read-codes-clear-confirm")),
                .thread = null
            },
            .actionWaitIcon = (GtkSpinner*)gtk_builder_get_object (builder, "window-read-codes-read-wait-icon"),
        };
        gtk_text_view_set_buffer(g.dtc.description, g.dtc.descriptionText);
        gtk_text_view_set_buffer(g.dtc.causeSolution, g.dtc.causeSolutionText);
        gtk_text_view_set_buffer(g.dtc.explanation, g.dtc.explanationText);
        gtk_text_view_set_buffer(g.menuBar.showECUsBuffer.textView, g.menuBar.showECUsBuffer.text);
        *tcgui = g;
        gtk_builder_add_callback_symbol(builder,"window-read-codes-go-back-main-menu",&read_codes_go_back_main_menu);
        g_signal_connect(G_OBJECT(tcgui->window),"delete-event",G_CALLBACK(read_codes_onclose),null);
        g_signal_connect(G_OBJECT(tcgui->clear.confirm),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        error_feedback_windows_init(tcgui->errorFeedback);
        g_signal_connect(G_OBJECT(tcgui->noObdData),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        g_signal_connect(G_OBJECT(tcgui->menuBar.showECUsBuffer.window),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        gtk_builder_add_callback_symbol(builder,"show-window-read-codes",&read_codes_show_window);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-read-codes",&trouble_code_reader_read_codes);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-clear-codes",&read_codes_show_clear_confirm);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-dtc-selected",G_CALLBACK(&trouble_code_reader_dtc_selected));
        gtk_builder_add_callback_symbol(builder,"window-read-codes-confirm-cancel",&trouble_code_reader_confirm_cancel);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-confirm-confirm",&trouble_code_reader_confirm_confirm);        
        gtk_builder_add_callback_symbol(builder,"window-read-codes-no-obd-interface-openned-ok",&trouble_code_reader_read_codes_error_popup_ok);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-no-serial-selected-ok",&trouble_code_reader_read_codes_error_serial_ok);
        gtk_builder_add_callback_symbol(builder,"window-no-obd-data-ok",&trouble_code_reader_read_codes_no_obd_data_ok);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-show-ecus-buffer",&trouble_code_reader_read_codes_show_ecus_buffer);
    } else {
        module_debug(MODULE_CODES_READER, "Module already initialized");
    }
}
