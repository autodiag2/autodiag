#include "ui/trouble_code_reader.h"

TroubleCodeReaderGui * tcgui = null;
pthread_t *trouble_code_reader_read_thread = null, * trouble_code_reader_clear_thread = null;
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
    gtk_widget_show_on_main_thread(tcgui->clearConfirm);
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
    gtk_widget_hide_on_main_thread(tcgui->noObdInterface);
}

void trouble_code_reader_read_codes_error_popup_show() {
    gtk_widget_show_on_main_thread(tcgui->noObdInterface);
}
void trouble_code_reader_read_codes_error_serial_ok() {
    gtk_widget_hide_on_main_thread(GTK_WIDGET(tcgui->serialError.window));
}
void trouble_code_reader_read_codes_no_obd_data_ok() {
    gtk_widget_hide_on_main_thread(tcgui->noObdData);
}
bool trouble_code_reader_read_codes_nonnull_with_error_feedback(final OBDIFACE iface) {
    if ( iface == null ) {
        final SERIAL serial = serial_list_get_selected();
        if ( serial == null || serial->status != SERIAL_STATE_READY ) {
            error_feedback_serial(tcgui->serialError.window,serial);
        } else {
            log_msg("Cannot read codes : no obd interface openned", LOG_INFO);
            trouble_code_reader_read_codes_error_popup_show();
        }
        return false;
    } else {
        if ( iface->status == SERIAL_STATE_READY ) {
            return true;
        } else {
            error_feedback_serial(tcgui->serialError.window,(SERIAL)iface);
            return false;
        }
    }
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
        for(int codei = 0; codei < dtc_list->size; codei++) {
            char * dtc_string = saej1979_dtc_to_string(dtc_list->list[codei]);
            GtkWidget *label = gtk_label_new(dtc_string);
            gtk_container_add((GtkContainer*)tcgui->dtc.list,label);
            gtk_widget_show(label);
            free(dtc_string);
        }
    }
    return false;
}

void trouble_code_reader_read_codes_daemon_internal() {
    final OBDIFACE iface = config.ephemere.iface;
    if ( trouble_code_reader_read_codes_nonnull_with_error_feedback(iface) ) {
        gtk_spinner_start(tcgui->actionWaitIcon);
        // The state of the port may change if thereis an error during the retrieve
        final bool mil = saej1979_get_current_mil_status(iface);
        if ( trouble_code_reader_read_codes_nonnull_with_error_feedback(iface) ) {
            {
                bool * mil_ptr = (bool*)malloc(sizeof(bool));
                *mil_ptr = mil;
                g_idle_add(trouble_code_reader_read_codes_set_mil_gsource, (gpointer)mil_ptr);
            }
            {
                char *dtcStr;
                final int dtcCount = saej1979_get_current_number_of_dtc_from_fetch(iface);
                if ( dtcCount == SAEJ1979_NUMBER_OF_DTC_ERROR ) {
                    dtcStr = strdup("0 DTC");
                    gtk_widget_show_on_main_thread(tcgui->noObdData);
                } else {
                    asprintf(&dtcStr,"%d DTC%s", dtcCount, 1 < dtcCount ? "s" : "" );
                }
                g_idle_add(trouble_code_reader_read_codes_set_dtc_count_gsource, (gpointer)dtcStr);
            }
            {
                if ( dtc_list != null ) {
                    saej1979_dtc_list_free(dtc_list);
                    dtc_list = null;
                }
                final SAEJ1979_DTC_list * pending = saej1979_dtc_list_new();
                if ( gtk_toggle_button_get_active(tcgui->read.stored) ) {
                    saej1979_dtc_list_append_list(pending,saej1979_retrieve_stored_dtcs(iface));
                }
                if ( gtk_toggle_button_get_active(tcgui->read.pending) ) {
                    saej1979_dtc_list_append_list(pending,saej1979_retrieve_pending_dtcs(iface));
                }
                if ( gtk_toggle_button_get_active(tcgui->read.permanent) ) {
                    saej1979_dtc_list_append_list(pending,saej1979_retrieve_permanent_dtcs(iface));
                }
                if ( 0 < pending->size ) {
                    dtc_list = pending;
                }
                g_idle_add(trouble_code_reader_read_codes_set_dtc_list_gsource, (gpointer)dtc_list);
            }
        }
    }
    trouble_code_reader_clear_dtc_description();
}

void* trouble_code_reader_read_codes_daemon(void *arg) {
    pthread_cleanup_push(trouble_code_reader_button_click_clean_up_routine, null);
    trouble_code_reader_read_codes_daemon_internal();
    free(trouble_code_reader_read_thread);
    trouble_code_reader_read_thread = null;
    pthread_cleanup_pop(1);
}

void trouble_code_reader_read_codes() {
    thread_allocate_and_start(&trouble_code_reader_read_thread, &trouble_code_reader_read_codes_daemon);
}

void* trouble_code_reader_clear_codes_daemon(void *arg) {
    pthread_cleanup_push(trouble_code_reader_button_click_clean_up_routine, null);
    gtk_spinner_start(tcgui->actionWaitIcon);
    final OBDIFACE iface = config.ephemere.iface;
    if ( trouble_code_reader_read_codes_nonnull_with_error_feedback(iface) ) {
        saej1979_clear_dtc_and_stored_values(iface);
    }
    trouble_code_reader_clear_dtc_description();
    free(trouble_code_reader_clear_thread);
    trouble_code_reader_clear_thread = null;
    gtk_widget_hide_on_main_thread(tcgui->clearConfirm);
    trouble_code_reader_read_codes_daemon_internal();
    pthread_cleanup_pop(1);
}

gboolean read_codes_onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused) {
    log_msg("Close event received", 0);
    if ( trouble_code_reader_read_thread != null ) {
        pthread_cancel(*trouble_code_reader_read_thread);
        free(trouble_code_reader_read_thread);
        trouble_code_reader_read_thread = null;
    }
    if ( trouble_code_reader_clear_thread != null ) {
        pthread_cancel(*trouble_code_reader_clear_thread);
        free(trouble_code_reader_clear_thread);
        trouble_code_reader_clear_thread = null;
    }
    return gtk_widget_generic_onclose(dialog,event,unused);
}

void trouble_code_reader_confirm_cancel() {
    gtk_widget_hide(tcgui->clearConfirm);
}
void trouble_code_reader_confirm_confirm() {
    thread_allocate_and_start(&trouble_code_reader_clear_thread,&trouble_code_reader_clear_codes_daemon);
}

void trouble_code_reader_append_multi_brand_explanation(GtkTextBuffer *buffer, SAEJ1979_DTC_DESCRIPTION *desc, char *part) {
    if ( part == null ) {

    } else {
        char *textEngine;
        if ( desc->car->engine == null || desc->car->engine->model == null ) {
            textEngine = strdup("");
        } else {
            asprintf(&textEngine, "(engine: %s)", desc->car->engine == null ? "" : desc->car->engine->model == null ? "" : desc->car->engine->model);
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
        SAEJ1979_DTC *dtc = saej1979_dtc_list_get(dtc_list,(char*)selectedDTC);

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
            .clearConfirm = GTK_WIDGET (gtk_builder_get_object (builder, "window-read-codes-clear-confirm")),
            .noObdInterface = GTK_WIDGET (gtk_builder_get_object (builder, "window-no-obd-interface-openned")),
            .noObdData = GTK_WIDGET (gtk_builder_get_object (builder, "window-no-obd-data")),
            .serialError = {
                .window = (GtkMessageDialog*) (gtk_builder_get_object (builder, "window-no-serial-selected"))
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
                .permanent = (GtkToggleButton*)gtk_builder_get_object (builder, "window-read-codes-read-permanent")
            },
            .actionWaitIcon = (GtkSpinner*)gtk_builder_get_object (builder, "window-read-codes-read-wait-icon")
        };
        gtk_text_view_set_buffer(g.dtc.description, g.dtc.descriptionText);
        gtk_text_view_set_buffer(g.dtc.causeSolution, g.dtc.causeSolutionText);
        gtk_text_view_set_buffer(g.dtc.explanation, g.dtc.explanationText);
        *tcgui = g;
        gtk_builder_add_callback_symbol(builder,"window-read-codes-go-back-main-menu",&read_codes_go_back_main_menu);
        g_signal_connect(G_OBJECT(tcgui->window),"delete-event",G_CALLBACK(read_codes_onclose),null);
        g_signal_connect(G_OBJECT(tcgui->clearConfirm),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        g_signal_connect(G_OBJECT(tcgui->noObdInterface),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        g_signal_connect(G_OBJECT(tcgui->serialError.window),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        g_signal_connect(G_OBJECT(tcgui->noObdData),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null);
        gtk_builder_add_callback_symbol(builder,"show-window-read-codes",&read_codes_show_window);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-read-codes",&trouble_code_reader_read_codes);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-clear-codes",&read_codes_show_clear_confirm);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-dtc-selected",G_CALLBACK(&trouble_code_reader_dtc_selected));
        gtk_builder_add_callback_symbol(builder,"window-read-codes-confirm-cancel",&trouble_code_reader_confirm_cancel);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-confirm-confirm",&trouble_code_reader_confirm_confirm);        
        gtk_builder_add_callback_symbol(builder,"window-read-codes-no-obd-interface-openned-ok",&trouble_code_reader_read_codes_error_popup_ok);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-no-serial-selected-ok",&trouble_code_reader_read_codes_error_serial_ok);
        gtk_builder_add_callback_symbol(builder,"window-no-obd-data-ok",&trouble_code_reader_read_codes_no_obd_data_ok);
    } else {
        module_debug(MODULE_CODES_READER, "Module already initialized");
    }
}
