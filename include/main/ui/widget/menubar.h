#ifndef __UI_LIB_MENUBAR_H
#define __UI_LIB_MENUBAR_H

#define MENUBAR_DATA_ALL_IN_ONE \
static void menubar_data_filter_by_on_filter_check_toggled(GtkCheckMenuItem *check_item, gpointer user_data){ \
    ECU *ecu=g_object_get_data(G_OBJECT(check_item),"ecu"); \
    if(gtk_check_menu_item_get_active(check_item))viface_recv_filter_add(config.ephemere.iface,ecu->address); \
    else viface_recv_filter_rm(config.ephemere.iface,ecu->address); \
} \
static GtkWidget* menubar_data_filter_by_find_widget_for_ecu(GtkWidget *menu, ECU *ecu){ \
    GList *children=gtk_container_get_children(GTK_CONTAINER(menu)); \
    for(GList *l=children;l!=NULL;l=l->next){ \
        GtkWidget *w=GTK_WIDGET(l->data); \
        ECU *attached=g_object_get_data(G_OBJECT(w),"ecu"); \
        if(attached==ecu){ \
            g_list_free(children); \
            return w; \
        } \
    } \
    g_list_free(children); \
    return null; \
} \
static void menubar_data_filter_by_filter_change(const char *type,Buffer *address){ \
    if(strcmp(type,"clear")==0){ \
        GList *children=gtk_container_get_children(GTK_CONTAINER(gui->menuBar.data.filter_by_menu)); \
        for(GList *l=children;l!=NULL;l=l->next){ \
            GtkWidget *w=GTK_WIDGET(l->data); \
            gtk_check_menu_item_set_active(w,false); \
        } \
        g_list_free(children); \
    }else{ \
        ECU *ecu=vehicle_search_ecu_by_address(config.ephemere.iface->vehicle,address); \
        if(ecu==null){ \
            log_msg(LOG_WARNING,"ecu with address '%s' not found",buffer_to_hex_string(address)); \
        }else{ \
            GtkWidget *w=menubar_data_filter_by_find_widget_for_ecu(gui->menuBar.data.filter_by_menu,ecu); \
            if(w==null){ \
                log_msg(LOG_WARNING,"widget associated with: not found"); \
                vehicle_ecu_debug(ecu); \
            }else gtk_check_menu_item_set_active(w,strcmp(type,"add")==0); \
        } \
    } \
} \
static void menubar_data_filter_by_register(const ECU *ecu){ \
    char *displayLabel; \
    asprintf(&displayLabel,"%s (%s)",ecu->name,buffer_to_hex_string(ecu->address)); \
    GtkWidget *filter_check=menubar_data_filter_by_find_widget_for_ecu(gui->menuBar.data.filter_by_menu,ecu); \
    if(filter_check==null){ \
        filter_check=gtk_check_menu_item_new_with_label(displayLabel); \
        gtk_menu_shell_append(GTK_MENU_SHELL(gui->menuBar.data.filter_by_menu),filter_check); \
        gtk_widget_show(filter_check); \
        g_object_set_data(G_OBJECT(filter_check),"ecu",(gpointer)ecu); \
        g_signal_connect(filter_check,"toggled",G_CALLBACK(menubar_data_filter_by_on_filter_check_toggled),gui->menuBar.data.all); \
    }else gtk_menu_item_set_label(GTK_MENU_ITEM(filter_check),displayLabel); \
    free(displayLabel); \
} \
static void menubar_data_all(GtkMenuItem *item,gpointer user_data){ \
    viface_recv_filter_clear(config.ephemere.iface); \
}

#define MENUBAR_DATA_CONNECT() \
    g_signal_connect(gui->menuBar.data.all, "activate", G_CALLBACK(menubar_data_all), null); \
    gui->menuBar.data.filter_by_menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(gui->menuBar.data.filter_by)); \
    ehh_register(config.ephemere.iface->vehicle->internal.events.onECURegister, menubar_data_filter_by_register); \
    ehh_register(config.ephemere.iface->vehicle->internal.events.onFilterChange, menubar_data_filter_by_filter_change); \

#endif