#include "libprog/elm327_cli.h"

PRINT_MODULAR(sim_elm327_cli_help,
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
    " -c context    : context for the generator\n"
    "\n"
    "Examples:\n"
    " elm327sim -g cycle -e EA -g random    : default ecu E8 cycle generator, EA ecu random generator\n"
    " elm327sim -g random -c 1234           : use the seed 1234 for generating random numbers\n"
    " elm327sim -g cycle -c 10              : number of gears used in the cycle\n"
    " elm327sim -g gui                      : default ecu with gui value generator\n"
    "\n"
)


void sim_elm327_cli_display_protocols() {
    printf("Supported protocols:\n");
    for(int p = 0x1; p <= 0xC; p += 1) {
        printf("%1x : %s\n", p, elm327_protocol_to_string(p));
    }
}

void sim_elm327_cli_display_help() {
    sim_elm327_cli_help("");
}

void sim_elm327_add_dtc(GtkButton *button, gpointer user_data) {
    ELM327SimGui* simGui = (ELM327SimGui*)user_data;
    const char *dtc_string = gtk_entry_get_text(simGui->dtcs.input);
    if ( saej1979_dtc_bin_from_string(dtc_string) == null ) {
        gtk_message_dialog_format_secondary_text(simGui->dtcs.invalidDtc,"%s: expected LXXXX where L is P,C,B,U",dtc_string);
        gtk_widget_show_on_main_thread(simGui->dtcs.invalidDtc);
    } else {
        GtkWidget *label = gtk_label_new(dtc_string);
        gtk_container_add((GtkContainer*)simGui->dtcs.listView,label);
        gtk_widget_show(label);
    }
}
ELM327SimGui * sim_elm327_build_gui(SimECUGenerator *generator, char * ecuDesignation) {

    gtk_init(0, NULL);

    char *ui_dir = installation_folder_resolve("ui"), *elm327simUiPath;
    if (ui_dir == NULL) {
        log_msg(LOG_ERROR, "Data directory not found, try reinstalling the software");
        return NULL;
    }
    asprintf(&elm327simUiPath, "%s" PATH_FOLDER_DELIM "elm327sim.glade", ui_dir);
    
    GtkBuilder *builder = gtk_builder_new_from_file(elm327simUiPath);
    free(ui_dir);
    free(elm327simUiPath);
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window-root"));
    char * title;
    asprintf(&title, "ECU %s generator", ecuDesignation);
    gtk_window_set_title(GTK_WINDOW(window), title);
    free(title);

    ELM327SimGui *simGui = (ELM327SimGui *)malloc(sizeof(ELM327SimGui));
    *simGui = (ELM327SimGui){
        .window = GTK_WIDGET(gtk_builder_get_object(builder, "window-root")),
        .dtcs = {
            .listView = GTK_LIST_BOX(gtk_builder_get_object(builder, "dtcs-list-view")),
            .input = GTK_ENTRY(gtk_builder_get_object(builder, "dtc-list-input")),
            .inputButton = GTK_BUTTON(gtk_builder_get_object(builder, "dtc-list-input-button")),
            .milOn = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "dtc-list-mil")),
            .dtcCleared = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "dtc-list-dtc-cleared")),
            .invalidDtc = GTK_WIDGET(gtk_builder_get_object(builder, "window-invalid-dtc"))
        },
        .data = {
            .vehicleSpeed = GTK_WIDGET(gtk_builder_get_object(builder, "data-vehicle-speed")),
            .coolantTemperature = GTK_WIDGET(gtk_builder_get_object(builder, "data-coolant-temperature")),
            .engineSpeed = GTK_WIDGET(gtk_builder_get_object(builder, "data-engine-speed"))
        }
    };

    g_signal_connect(G_OBJECT(simGui->window), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), NULL);
    g_signal_connect(G_OBJECT(simGui->dtcs.invalidDtc), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), NULL);
    g_signal_connect(simGui->dtcs.inputButton, "clicked", G_CALLBACK(sim_elm327_add_dtc), simGui);

    counter_init_modifiable(simGui->data.vehicleSpeed,"counter_85_2_255_0_0_255.png", true);
    counter_init_modifiable(simGui->data.coolantTemperature,"gaugehalf_225_5_255_0_0_255.png", true);
    counter_init_modifiable(simGui->data.engineSpeed,"counter_85_2_255_0_0_255.png", true);

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));

    generator->context = (void *)simGui;

    return simGui;
}

LIST_H(ELM327SimGui)
LIST_SRC(ELM327SimGui)

typedef struct {
    SimELM327* sim;
    ELM327_PROTO *proto;
    bool * proto_is_auto;
} ELM327SimData;

void *sim_elm327_daemon(void *d) {
    ELM327SimData* da = d;
    ELM327SimData data = *da;
    sim_elm327_loop_as_daemon(data.sim);
    usleep(50e3);
    if ( data.sim->device_location == null ) {
        log_msg(LOG_WARNING, "Simulation not started");
    } else {
        printf("Simulation running on %s\n", data.sim->device_location);
        if ( data.proto != null ) {
            data.sim->protocolRunning = *data.proto;
        }
        if ( data.proto_is_auto != null ) {
            data.sim->protocol_is_auto_running = *data.proto_is_auto;
        }
        if ( data.sim->implementation->loop_thread != null ) {
            pthread_join(*data.sim->implementation->loop_thread, NULL);
        }
    }
    return null;
}
gboolean sim_elm327_present_window(gpointer w) {
    sleep(1);
    gtk_window_set_keep_above(GTK_WINDOW(w), false);
    return false;
}
int sim_elm327_cli_main(int argc, char **argv) {
    SimELM327* sim = sim_elm327_new();
    ELM327_PROTO *proto = null;
    bool * proto_is_auto = null;
    ELM327SimGui_list * guis = ELM327SimGui_list_new();
    
    int opt;
    optind = 1;
    while ((opt = getopt(argc, argv, "he:l:p:g:c:")) != -1) {
        switch (opt) {
            case 'h': {
                sim_elm327_cli_display_help();
                return 0;
            }
            case 'e': {
                byte ecu_address;
                if ( sscanf(optarg,"%02hhx", &ecu_address) == 1 ) {
                    SimECU_list_append(sim->ecus,sim_ecu_emulation_new(ecu_address));                    
                } else {
                    sim_elm327_cli_display_help();
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
                    sim_elm327_cli_display_help();
                    return 1;
                }
            } break;
            case 'l': {
                logger.current_level = log_level_from_str(optarg);
            } break;
            case 'g': {
                final SimECU * target_ecu = sim->ecus->list[sim->ecus->size - 1];
                final SimECUGenerator * generator;
                if ( strcasecmp(optarg, "random") == 0 ) {
                    generator = sim_ecu_generator_new_random();
                } else if ( strcasecmp(optarg, "cycle") == 0 ) {
                    generator = sim_ecu_generator_new_cycle();
                } else if ( strcasecmp(optarg,"citroen_c5_x7") == 0 ) {
                    generator = sim_ecu_generator_new_citroen_c5_x7();
                } else if ( strcasecmp(optarg,"gui") == 0 ) {
                    generator = sim_ecu_generator_new_gui();
                    char address[3];
                    sprintf(address, "%02hhX", target_ecu->address);
                    ELM327SimGui * context = sim_elm327_build_gui(generator, address);
                    generator->context = context;
                    ELM327SimGui_list_append(guis, context);
                } else {
                    log_msg(LOG_ERROR, "Unknown generator %s", optarg);
                    return 1;
                }
                generator->type = strdup(optarg);
                target_ecu->generator = generator;
            } break;
            case 'c': {
                final SimECUGenerator *generator = sim->ecus->list[sim->ecus->size - 1]->generator;
                if ( strcasecmp(generator->type, "random") == 0 ) {
                    unsigned *context = (unsigned*)malloc(sizeof(unsigned));
                    generator->context = context; 
                    if ( sscanf(optarg, "%u", context) != 1 ) {
                        printf("Expected unsigned int context for random generator, context is used as the seed\n");
                        return 1;
                    }
                } else if ( strcasecmp(generator->type, "cycle") == 0 ) {
                    unsigned *context = (unsigned*)malloc(sizeof(unsigned));
                    generator->context = context; 
                    if ( sscanf(optarg, "%u", context) != 1 ) {
                        printf("Expected unsigned int number of gears for cycle generator\n");
                        return 1;
                    }
                } else {
                    printf("Generator type %s as no user definable context\n", generator->type);
                    return 1;
                }
            } break;
            case '?': {
                switch ( optopt ) {
                    case 'p':
                        sim_elm327_cli_display_protocols();
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
                        printf("citroen_c5_x7\n");
                        printf("gui\n");
                        break;
                    case 'c': {
                        printf("give the context to the -c, cannot be empty\n");
                    } break;
                }
                return 1;
            }
        }
    }
    for(int i = 0; i < guis->size; i ++) {
        ELM327SimGui *simGui = guis->list[i];
        gtk_widget_show(simGui->window);
        gtk_window_set_keep_above(GTK_WINDOW(simGui->window), true);
        g_idle_add(sim_elm327_present_window, (gpointer)simGui->window);
        gtk_window_present(GTK_WINDOW(simGui->window));
    }

    ELM327SimData data = {
        .sim = sim,
        .proto = proto,
        .proto_is_auto = proto_is_auto
    };
    pthread_t simThread;
    pthread_create(&simThread, null, &sim_elm327_daemon, &data);

    if ( 0 < guis->size ) {
        gtk_main();
    } else {
        pthread_join(simThread, null);
    }

    ELM327SimGui_list_free(guis);
    return 0;
}

