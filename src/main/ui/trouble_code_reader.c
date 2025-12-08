#include "ui/trouble_code_reader.h"

TroubleCodeReaderGui * gui = null;
list_DTC * list_dtc = null;

MENUBAR_DATA_ALL_IN_ONE

static void clear_dtc_description() {
    gtk_text_buffer_set_text(gui->dtc.explanationText, "", 0);
    gtk_text_buffer_set_text(gui->dtc.descriptionText, "", 0);
    gtk_text_buffer_set_text(gui->dtc.causeSolutionText, "", 0);
}
static bool filtered_dtc_state() {
    return gtk_check_menu_item_get_active(gui->menuBar.filtered);    
}
static void hide() {
    gtk_widget_hide_on_main_thread(gui->window);
}

static void show() {
    gtk_window_show_ensure_ontop(gui->window);
}

static void show_clear_confirm() {
    gtk_widget_show_on_main_thread(gui->clear.confirm);
}

static void button_click_clean_up_routine(void *arg) {
    obd_thread_cleanup_routine(arg);
    gtk_spinner_stop(gui->actionWaitIcon);
}

static void error_popup_ok() {
    gtk_widget_hide_on_main_thread(gui->errorFeedback.obd);
}
static void error_serial_ok() {
    gtk_widget_hide_on_main_thread(GTK_WIDGET(gui->errorFeedback.serial));
}
static void no_obd_data_ok() {
    gtk_widget_hide_on_main_thread(gui->noObdData);
}
static gboolean show_ecus_buffer_gsource(gpointer data) {
    char *msg = (char*)data; 
    gtk_text_buffer_set_text(gui->menuBar.showECUsBuffer.text, msg, strlen(msg));
    free(msg);
    return false;
}
static void show_ecus_buffer_internal() {
    final VehicleIFace* iface = config.ephemere.iface;
    char * result = strdup("");
    if ( iface->state == VIFaceState_READY ) {
        Vehicle * vehicle = iface->vehicle;    
        for(int i = 0; i < vehicle->ecus_len; i++) {
            ECU* ecu = vehicle->ecus[i];
            {
                char *tmp;
                char * ascii_dump = buffer_to_hex_string(ecu->address);
                asprintf(&tmp,"%sOn ECU '%s' at address: %s\n",result,ecu->name == null ? "" : ecu->name,ascii_dump);
                free(result);
                result = tmp;
                free(ascii_dump);
            }
            
            AD_LIST_FOREACH(ecu->data_buffer,Buffer,buffer,
                char * dump = bytes_to_hexdump(buffer->buffer, buffer->size);
                char *tmp;
                asprintf(&tmp, "%sdata#%d:\n%s",result,list_element_index+1,dump);
                free(result);
                result = tmp;
                free(dump);
            )
        }
    }
    g_idle_add(show_ecus_buffer_gsource, (gpointer)result);
}

THREAD_WRITE_DAEMON(
        show_ecus_buffer_daemon,
        show_ecus_buffer_internal,
        button_click_clean_up_routine, gui->menuBar.showECUsBuffer.thread
)

static void show_ecus_buffer() {
    gtk_widget_show_on_main_thread(gui->menuBar.showECUsBuffer.window);
    thread_allocate_and_start(&gui->menuBar.showECUsBuffer.thread,&show_ecus_buffer_daemon);
}

static gboolean set_mil_gsource(gpointer data) {
    bool mil = *((bool*)data);
    gtk_label_set_text(gui->mil.text, mil ? "MIL is ON":"MIL is OFF");
    gtk_widget_set_visible((GtkWidget*)gui->mil.iconOn,mil);
    gtk_widget_set_visible((GtkWidget*)gui->mil.iconOff,!mil);
    free(data);
    return false;
}

static gboolean set_dtc_count_gsource(gpointer data) {
    char *dtcStr = (char*)data;
    gtk_label_set_text(gui->dtc.count, dtcStr);
    free(dtcStr);
    return false;
}
static gboolean set_list_dtc_gsource(gpointer data) {
    list_DTC *list_dtc = (list_DTC *)data;
    GList *ptr = gtk_container_get_children((GtkContainer*)gui->dtc.list);
    while(ptr != null) {
        final GList *ptr_next = ptr->next;
        gtk_widget_destroy(ptr->data);
        ptr = ptr_next;
    }
    if ( list_dtc != null ) {
        AD_LIST_FOREACH(list_dtc,DTC,dtc,
            char * dtc_string = dtc->to_string(CAST_DTC(dtc));
            GtkWidget *label = gtk_label_new(dtc_string);
            g_object_set_data(G_OBJECT(label), "dtc", dtc);
            gtk_container_add((GtkContainer*)gui->dtc.list,label);
            gtk_widget_show(label);
            free(dtc_string);
        )
    }
    return false;
}

static void read_codes_daemon_internal() {
    final VehicleIFace* iface = config.ephemere.iface;
    if ( ! trouble_code_reader_error_feedback_obd(iface) ) {
        gtk_spinner_start(gui->actionWaitIcon);
        // The state of the port may change if thereis an error during the retrieve
        if ( ! trouble_code_reader_error_feedback_obd(iface) ) {
            if ( gtk_check_menu_item_get_active(gui->menuBar.data.obd.origin) ) {
                final bool mil = saej1979_data_mil_status(iface,false);
                bool * mil_ptr = (bool*)malloc(sizeof(bool));
                *mil_ptr = mil;
                g_idle_add(set_mil_gsource, (gpointer)mil_ptr);
            }
            {
                if ( list_dtc != null ) {
                    list_DTC_free(list_dtc);
                    list_dtc = null;
                }
                final list_DTC * list_dtc_buffer = list_DTC_new();
                final list_DTC * list_dtc_obd = list_DTC_new();
                final list_DTC * list_dtc_uds = list_DTC_new();
                bool state = filtered_dtc_state();
                final Vehicle* filter = state ? iface->vehicle : null;
                if ( gtk_check_menu_item_get_active(gui->menuBar.data.obd.origin) ) {
                    if ( gtk_check_menu_item_get_active(gui->menuBar.data.obd.stored) ) {
                        list_DTC_append_list(list_dtc_obd,saej1979_retrieve_stored_dtcs(iface, filter));
                    }
                    if ( gtk_check_menu_item_get_active(gui->menuBar.data.obd.pending) ) {
                        list_DTC_append_list(list_dtc_obd,saej1979_retrieve_pending_dtcs(iface, filter));
                    }
                    if ( gtk_check_menu_item_get_active(gui->menuBar.data.obd.permanent) ) {
                        list_DTC_append_list(list_dtc_obd,saej1979_retrieve_permanent_dtcs(iface, filter));
                    }
                }
                if ( gtk_check_menu_item_get_active(gui->menuBar.data.uds.origin) ) {
                    list_DTC_append_list(list_dtc_uds, (list_DTC*)uds_read_all_dtcs(iface, filter));
                    for(unsigned int i = 0; i < list_dtc_uds->size; i++) {
                        final DTC * dtc = list_dtc_uds->list[i];
                        if ( list_DTC_contains(list_dtc_obd, dtc) ) {
                            list_object_string_append(dtc->detection_method, object_string_new_from("OBD"));
                        }
                    }
                    list_DTC_append_list(list_dtc_buffer, list_dtc_uds);
                }
                if ( gtk_check_menu_item_get_active(gui->menuBar.data.obd.origin) ) {
                    for(unsigned int i = 0; i < list_dtc_obd->size; i ++) {
                        final DTC * dtc = list_dtc_obd->list[i];
                        if ( ! list_DTC_contains(list_dtc_buffer, dtc)) {
                            list_DTC_append(list_dtc_buffer, dtc);
                        }
                    }
                }
                list_DTC_free(list_dtc_obd);
                list_DTC_free(list_dtc_uds);
                if ( 0 < list_dtc_buffer->size ) {
                    list_dtc = list_dtc_buffer;
                }
                g_idle_add(set_list_dtc_gsource, (gpointer)list_dtc);
            }
            {
                int dtcCount = 0;
                if ( list_dtc != null ) {
                    dtcCount = list_dtc->size;
                }
                char *dtcStr;
                asprintf(&dtcStr,"%d DTC%s", dtcCount, 1 < dtcCount ? "s" : "" );
                g_idle_add(set_dtc_count_gsource, (gpointer)dtcStr);
            }
        }
    }
    clear_dtc_description();
}

THREAD_WRITE_DAEMON(
        read_codes_daemon, read_codes_daemon_internal,
        button_click_clean_up_routine, gui->read.thread
)

static void read_codes() {
    thread_allocate_and_start(&gui->read.thread, &read_codes_daemon);
}

static void clear_codes_daemon_internal() {
    gtk_spinner_start(gui->actionWaitIcon);
    final VehicleIFace* iface = config.ephemere.iface;
    if ( ! trouble_code_reader_error_feedback_obd(iface) ) {
        bool at_least_one = false;
        if ( gtk_check_menu_item_get_active(gui->menuBar.data.obd.origin) ) {
            saej1979_clear_dtc_and_stored_values(iface);
            at_least_one = true;
        }
        if ( gtk_check_menu_item_get_active(gui->menuBar.data.uds.origin) ) {
            if ( uds_clear_dtcs(iface) ) {
                log_msg(LOG_DEBUG, "DTCs(UDS) successfully cleared");
            } else {
                log_msg(LOG_ERROR, "Error while clearing DTCs(UDS)");
            }
            at_least_one = true;
        }
        if ( ! at_least_one ) {
            log_msg(LOG_WARNING, "Data configuration do not allow to send the request");
        }
    }
    gtk_widget_hide_on_main_thread(gui->clear.confirm);
    clear_dtc_description();
    read_codes_daemon_internal();
}
THREAD_WRITE_DAEMON(
        clear_codes_daemon, clear_codes_daemon_internal,
        button_click_clean_up_routine, gui->clear.thread
)

static gboolean onclose(GtkWidget *dialog, GdkEvent *event, gpointer unused) {
    log_msg(LOG_DEBUG, "Close event received");
    THREAD_CANCEL(gui->read.thread);
    THREAD_CANCEL(gui->clear.thread);
    THREAD_CANCEL(gui->menuBar.showECUsBuffer.thread);
    return gtk_widget_generic_onclose(dialog,event,unused);
}

static void confirm_cancel() {
    gtk_widget_hide(gui->clear.confirm);
}
static void confirm_confirm() {
    thread_allocate_and_start(&gui->clear.thread,&clear_codes_daemon);
}

static void append_multi_manufacturer_explanation(GtkTextBuffer *buffer, DTC_DESCRIPTION *desc, char *part) {
    if ( part == null ) {

    } else {
        char *textECU = strdup("(ecu: TODO)");
        char *text;
        asprintf(&text, " %s %s %s\n\t%s\n", desc->vehicle->manufacturer, desc->vehicle->engine == null ? "" : desc->vehicle->engine, textECU, part);
        free(textECU);
        GtkTextIter iter;
        gtk_text_buffer_get_iter_at_offset(buffer,&iter,-1);
        gtk_text_buffer_insert(buffer, &iter,text, -1);
        free(text);
    }
}

static void dtc_selected(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    clear_dtc_description();
    if ( row != null ) {
        final GtkLabel* rowLabel = GTK_LABEL(gtk_bin_get_child((GtkBin*)row));
        final DTC *dtc = (DTC*)g_object_get_data(G_OBJECT(rowLabel), "dtc");
        assert(dtc != null);

        for(unsigned int i = 0; i < dtc->description->size; i++) {
            DTC_DESCRIPTION desc = dtc->description->list[i];
            append_multi_manufacturer_explanation(gui->dtc.causeSolutionText,&desc,desc.solution);
            append_multi_manufacturer_explanation(gui->dtc.descriptionText,&desc,desc.reason);
        }
        final char * explanation = strdup("Detection methods: ");
        for(unsigned int i = 0; i < dtc->detection_method->size; i++) {
            char * dm = dtc->detection_method->list[i]->data;
            char * result;
            asprintf(&result, "%s%s ", explanation, dm);
            free(explanation);
            explanation = result;
        }
        {
            char * result;
            asprintf(&result, "%s\n%s", explanation, saej1979_dtc_explanation(dtc));
            free(explanation);
            explanation = result;
        }
        if ( list_object_string_contains(dtc->detection_method, object_string_new_from("UDS")) ) {
            char * result;
            asprintf(&result, "%s%s", explanation, UDS_DTC_explanation((UDS_DTC*)dtc));
            free(explanation);
            explanation = result;
        }
        gtk_text_buffer_set_text(gui->dtc.explanationText,explanation,strlen(explanation));
        free(explanation);
    }
}
void module_init_read_codes(GtkBuilder *builder) {
    if ( gui == null ) {
        module_debug_init(MODULE_CODES_READER);
        gui = (TroubleCodeReaderGui*)malloc(sizeof(TroubleCodeReaderGui));
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
                },
                .filtered = (GtkCheckMenuItem*) (gtk_builder_get_object (builder, "window-read-codes-view-filtered")),
                .data = {
                    .all = GTK_MENU_ITEM(gtk_builder_get_object(builder, "read-codes-menubar-data-source-all")),
                    .filter_by = GTK_WIDGET(gtk_builder_get_object(builder, "read-codes-menubar-data-source-filter-by")),
                    .obd = {
                        .origin = GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder,"read-codes-data-source-obd")),
                        .stored = GTK_CHECK_MENU_ITEM(gtk_builder_get_object (builder, "window-read-codes-read-stored")),
                        .pending = GTK_CHECK_MENU_ITEM(gtk_builder_get_object (builder, "window-read-codes-read-pending")),
                        .permanent = GTK_CHECK_MENU_ITEM(gtk_builder_get_object (builder, "window-read-codes-read-permanent")),
                    },
                    .uds = {
                        .origin = GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder, "read-codes-data-source-uds"))
                    }
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
        *gui = g;
        gtk_check_menu_item_set_active(gui->menuBar.filtered, false);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-go-back-main-menu",&hide);
        assert(0 != g_signal_connect(G_OBJECT(gui->window),"delete-event",G_CALLBACK(onclose),null));
        assert(0 != g_signal_connect(G_OBJECT(gui->clear.confirm),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null));
        error_feedback_windows_init(gui->errorFeedback);
        assert(0 != g_signal_connect(G_OBJECT(gui->noObdData),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null));
        assert(0 != g_signal_connect(G_OBJECT(gui->menuBar.showECUsBuffer.window),"delete-event",G_CALLBACK(gtk_widget_generic_onclose),null));
        gtk_builder_add_callback_symbol(builder,"show-window-read-codes",&show);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-read-codes",&read_codes);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-clear-codes",&show_clear_confirm);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-dtc-selected",G_CALLBACK(&dtc_selected));
        gtk_builder_add_callback_symbol(builder,"window-read-codes-confirm-cancel",&confirm_cancel);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-confirm-confirm",&confirm_confirm);        
        gtk_builder_add_callback_symbol(builder,"window-read-codes-no-obd-interface-openned-ok",&error_popup_ok);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-no-serial-selected-ok",&error_serial_ok);
        gtk_builder_add_callback_symbol(builder,"window-no-obd-data-ok",&no_obd_data_ok);
        gtk_builder_add_callback_symbol(builder,"window-read-codes-show-ecus-buffer",&show_ecus_buffer);

        gtk_check_menu_item_set_active(gui->menuBar.data.obd.origin, true);
        gtk_check_menu_item_set_active(gui->menuBar.data.obd.stored, true);
        gtk_check_menu_item_set_active(gui->menuBar.data.uds.origin, true);

        MENUBAR_DATA_CONNECT();
        
    } else {
        module_debug(MODULE_CODES_READER, "Module already initialized");
    }
}
