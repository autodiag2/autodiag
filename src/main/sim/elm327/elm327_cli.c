#include "sim/elm327/elm327_cli.h"

PRINT_MODULAR(elm327_sim_cli_help,
    "\n"
    "ELM327 simulator\n"
    "\n"
    " -h            : display this help\n"
    " -e hh         : add an ecu to the simulation with address hh\n"
    " -p            : list protocols\n"
    " -p h          : set protocol to h\n"
    " -p Ah         : set protocol to automatic, h\n"
    " -l            : list level of logging\n"
    " -l level      : set level of logging\n"
    " -g            : list available generators\n"
    " -g generator  : set the generator of values\n"
    "\n"
    "Examples:\n"
    " elm327sim -g cycle -e EA -g random : default ecu E8 cycle generator, EA ecu random generator\n"
)


void elm327_sim_cli_display_protocols() {
    printf("Supported protocols:\n");
    for(int p = 0x1; p <= 0xC; p += 1) {
        printf("%1x : %s\n", p, elm327_protocol_to_string(p));
    }
}

void elm327_sim_cli_display_help() {
    elm327_sim_cli_help("");
}

void elm327_sim_add_dtc(GtkButton *button, gpointer user_data) {
    ELM327SimGui* simGui = (ELM327SimGui*)user_data;
    char *dtc_string = gtk_entry_get_text(simGui->dtcs.input);
    GtkWidget *label = gtk_label_new(dtc_string);
    gtk_container_add((GtkContainer*)simGui->dtcs.listView,label);
    gtk_widget_show(label);
}

void *gtk_launch_ecu_generator_gui(void *data) {
    ECUEmulationGenerator *generator = (ECUEmulationGenerator *)data;

    gtk_init(0, NULL);

    char *ui_dir = config_get_in_data_folder_safe("ui"), *elm327simUiPath;
    if (ui_dir == NULL) {
        log_msg(LOG_ERROR, "Data directory not found, try reinstalling the software");
        return NULL;
    }
    asprintf(&elm327simUiPath, "%s" PATH_FOLDER_DELIM "elm327sim.glade", ui_dir);
    
    GtkBuilder *builder = gtk_builder_new_from_file(elm327simUiPath);
    free(ui_dir);
    free(elm327simUiPath);

    ELM327SimGui *simGui = (ELM327SimGui *)malloc(sizeof(ELM327SimGui));
    *simGui = (ELM327SimGui){
        .window = GTK_WIDGET(gtk_builder_get_object(builder, "window-root")),
        .dtcs = {
            .listView = GTK_LIST_BOX(gtk_builder_get_object(builder, "dtcs-list-view")),
            .input = GTK_ENTRY(gtk_builder_get_object(builder, "dtc-list-input")),
            .inputButton = GTK_BUTTON(gtk_builder_get_object(builder, "dtc-list-input-button")),
            .milOn = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "dtc-list-mil"))
        }
    };

    g_signal_connect(G_OBJECT(simGui->window), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), NULL);
    g_signal_connect(simGui->dtcs.inputButton, "clicked", G_CALLBACK(elm327_sim_add_dtc), simGui);

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));

    gtk_widget_show(simGui->window);
    gtk_window_set_keep_above(GTK_WINDOW(simGui->window), TRUE);
    gtk_window_present(GTK_WINDOW(simGui->window));

    generator->seed = (void *)simGui;

    log_msg(LOG_DEBUG, "GTK GUI initialized in a separate thread.");

    gtk_main();  // Run the GTK main loop in the new thread

    return NULL;
}

int elm327_sim_cli_main(int argc, char **argv) {
    ELM327emulation* sim = elm327_sim_new();
    ELM327_PROTO *proto = null;
    bool * proto_is_auto = null;
    
    int opt;
    optind = 1;
    while ((opt = getopt(argc, argv, "he:l:p:g:")) != -1) {
        switch (opt) {
            case 'h': {
                elm327_sim_cli_display_help();
                return 0;
            }
            case 'e': {
                byte ecu_address;
                if ( sscanf(optarg,"%02hhx", &ecu_address) == 1 ) {
                    ECUEmulation_list_append(sim->ecus,ecu_emulation_new(ecu_address));                    
                } else {
                    elm327_sim_cli_display_help();
                    return 1;
                }
            } break;
            case 'p': {
                int p;
                if ( sscanf(optarg,"A%1x", &p) == 1 ) {
                    proto = (ELM327_PROTO *)intdup(p);
                    proto_is_auto = intdup(true);
                } else if ( sscanf(optarg,"%1x", &p) == 1 ) {
                    proto = (ELM327_PROTO *)intdup(p);
                    proto_is_auto = intdup(false);
                } else {
                    elm327_sim_cli_display_help();
                    return 1;
                }
            } break;
            case 'l': {
                logger.current_level = log_level_from_str(optarg);
            } break;
            case 'g': {
                sim->ecus->list[sim->ecus->size - 1]->generator.type = ecu_sim_generator_from_string(optarg);
                ECUEmulationGenerator *generator = &(sim->ecus->list[sim->ecus->size - 1]->generator);

                if (generator->type == ECUEmulationGeneratorTypeGui) {
                    g_thread_new("Gui generator", gtk_launch_ecu_generator_gui, generator);
                }
            } break;
            case '?': {
                switch ( optopt ) {
                    case 'p':
                        elm327_sim_cli_display_protocols();
                        break;
                    case 'l':
                        printf("log levels:\n");
                        printf("none\n");
                        printf("error\n");                        
                        printf("warning\n");                        
                        printf("info\n");                        
                        printf("debug\n");    
                        break; 
                    case 'e':
                        printf("example: -e E8\n");                   
                        break;
                    case 'g':
                        printf("Available generators:\n");
                        printf("random\n");
                        printf("cycle\n");
                        printf("gui\n");
                        break;
                }
                return 1;
            }
        }
    }
    elm327_sim_loop_start(sim);
    usleep(50e3);
    if ( sim->port_name == null ) {
        log_msg(LOG_INFO, "Simulation not started");
        return 1;
    } else {
        printf("Simulation running on %s\n", sim->port_name);
        if ( proto != null ) {
            sim->protocolRunning = *proto;
        }
        if ( proto_is_auto != null ) {
            sim->protocol_is_auto_running = *proto_is_auto;
        }
        pthread_join(sim->loop_thread, NULL);
        return 0;
    }
}

