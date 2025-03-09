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
    " -c context    : context for the generator\n"
    "\n"
    "Examples:\n"
    " elm327sim -g cycle -e EA -g random    : default ecu E8 cycle generator, EA ecu random generator\n"
    " elm327sim -g random -c 1234           : use the seed 1234 for generating random numbers\n"
    " elm327sim -g cycle -c 10              : number of gears used in the cycle\n"
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
    if ( saej1979_dtc_bin_from_string(dtc_string) == null ) {
        gtk_message_dialog_format_secondary_text(simGui->dtcs.invalidDtc,"%s: expected LXXXX where L is P,C,B,U",dtc_string);
        gtk_widget_show_on_main_thread(simGui->dtcs.invalidDtc);
    } else {
        GtkWidget *label = gtk_label_new(dtc_string);
        gtk_container_add((GtkContainer*)simGui->dtcs.listView,label);
        gtk_widget_show(label);
    }
}
ELM327SimGui * elm327_sim_build_gui(ECUEmulationGenerator *generator) {

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
            .milOn = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "dtc-list-mil")),
            .dtcCleared = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "dtc-list-dtc-cleared")),
            .invalidDtc = GTK_WIDGET(gtk_builder_get_object(builder, "window-invalid-dtc"))
        },
        .data = {
            .vehicleSpeed = GTK_SCALE_BUTTON(gtk_builder_get_object(builder, "data-vehicle-speed")),
            .coolantTemperature = GTK_SCALE_BUTTON(gtk_builder_get_object(builder, "data-coolant-temperature")),
            .engineSpeed = GTK_SCALE_BUTTON(gtk_builder_get_object(builder, "data-engine-speed"))
        }
    };

    g_signal_connect(G_OBJECT(simGui->window), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), NULL);
    g_signal_connect(G_OBJECT(simGui->dtcs.invalidDtc), "delete-event", G_CALLBACK(gtk_widget_generic_onclose), NULL);
    g_signal_connect(simGui->dtcs.inputButton, "clicked", G_CALLBACK(elm327_sim_add_dtc), simGui);

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));

    generator->context = (void *)simGui;

    return simGui;
}

LIST_DEFINE_WITH_MEMBERS_AUTO(ELM327SimGui)
LIST_DEFINE_MEMBERS_SYM_AUTO(ELM327SimGui)
typedef struct {
    ELM327emulation* sim;
    ELM327_PROTO *proto;
    bool * proto_is_auto;
} ELM327SimData;

void *elm327_sim_daemon(void *d) {
    ELM327SimData* da = d;
    ELM327SimData data = *da;
    elm327_sim_loop_start(data.sim);
    usleep(50e3);
    if ( data.sim->port_name == null ) {
        log_msg(LOG_WARNING, "Simulation not started");
    } else {
        printf("Simulation running on %s\n", data.sim->port_name);
        if ( data.proto != null ) {
            data.sim->protocolRunning = *data.proto;
        }
        if ( data.proto_is_auto != null ) {
            data.sim->protocol_is_auto_running = *data.proto_is_auto;
        }
        pthread_join(data.sim->loop_thread, NULL);
    }
}

int elm327_sim_cli_main(int argc, char **argv) {
    ELM327emulation* sim = elm327_sim_new();
    ELM327_PROTO *proto = null;
    bool * proto_is_auto = null;
    ELM327SimGui_list * guis = ELM327SimGui_list_new();
    
    int opt;
    optind = 1;
    while ((opt = getopt(argc, argv, "he:l:p:g:c:")) != -1) {
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
                final ECUEmulationGeneratorType type = ecu_sim_generator_from_string(optarg);
                final ECUEmulationGenerator *generator = &(sim->ecus->list[sim->ecus->size - 1]->generator);
                generator->type = type;
                if (generator->type == ECUEmulationGeneratorTypeGui) {
                    ELM327SimGui_list_append(guis, elm327_sim_build_gui(generator));
                }
            } break;
            case 'c': {
                final ECUEmulationGenerator *generator = &(sim->ecus->list[sim->ecus->size - 1]->generator);
                switch (generator->type) {
                    case ECUEmulationGeneratorTypeRandom: {
                        unsigned *context = (unsigned*)malloc(sizeof(unsigned));
                        generator->context = context; 
                        if ( sscanf(optarg, "%u", context) != 1 ) {
                            printf("Expected unsigned int context for random generator, context is used as the seed\n");
                            return 1;
                        }
                    } break;
                    case ECUEmulationGeneratorTypeCycle: {
                        unsigned *context = (unsigned*)malloc(sizeof(unsigned));
                        generator->context = context; 
                        if ( sscanf(optarg, "%u", context) != 1 ) {
                            printf("Expected unsigned int number of gears for cycle generator\n");
                            return 1;
                        }
                    } break;
                    default: {
                        printf("Generator type %d as no user definalbe context\n", generator->type);
                    } break;
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
    }

    ELM327SimData data = {
        .sim = sim,
        .proto = proto,
        .proto_is_auto = proto_is_auto
    };
    pthread_t simThread;
    pthread_create(&simThread, null, &elm327_sim_daemon, &data);

    if ( 0 < guis->size ) {
        gtk_main();
    } else {
        pthread_join(simThread, null);
    }

    ELM327SimGui_list_free(guis);
}

