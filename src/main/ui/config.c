#include "ui/config.h"
#include "com/serial/obd/obd.h"

Config config = {
                    .com = {
                        .serial = {
                            .baud_rate = BAUD_RATE_UNDEFINED,
                            .port_name = null
                        }
                    },
                    .main = {
                        .adaptater_detailled_settings_showned = false
                    },
                    .commandLine = {
                        .autoScrollEnabled = true,
                        .showTimestamp = false
                    },
                    .ephemere = {
                        .iface = null
                    },
                    .vehicleExplorer = {
                        .refreshRateS = 0.1
                    }
                };

void config_commandLine_showTimestamp_set(final bool state) {
    config.commandLine.showTimestamp = state;
    logger.show_timestamp = state;
}

void config_dump(Config * config) {
    printf("Config {\n");
    printf("    com.serial {\n");
    printf("        baud_rate=%d, port_name=%s\n", config->com.serial.baud_rate, config->com.serial.port_name);
    printf("    }\n");
    printf("    main.adaptater_detailled_settings_showned=%d\n",config->main.adaptater_detailled_settings_showned);
    printf("    commandLine {\n");
    printf("        autoScrollEnabled=%d, showTimestamp=%d\n", config->commandLine.autoScrollEnabled, config->commandLine.showTimestamp);
    printf("    }\n");
    printf("}\n");
}

char *config_get_config_directory_top_internal() {
    char *path;
    asprintf(&path,"%s/.config/", getenv("HOME"));
    return path;
}

char *config_get_config_directory_internal() {
    char *path;
    asprintf(&path,"%s/.config/%s/", getenv("HOME"), APP_NAME);
    return path;
}

char *config_get_config_filename() {
    char *path;
    asprintf(&path,"%s/.config/%s/%s", getenv("HOME"), APP_NAME, CONFIG_DEFAULT_FILENAME);
    return path;
}
bool config_try_make_config_directory_one_dir(char *dir) {
    bool rv = true;
    if ( access(dir,R_OK|W_OK|X_OK) != 0 ) {
        if ( mkdir(dir,S_IRWXU) != 0 ) {
            if ( chmod(dir,S_IRWXU) != 0 ) {
                rv = false;
            }
        }
    }
    free(dir);
    return rv;
}

char *config_get_config_file_safe() {
    char *path = null;
    #ifdef POSIX_TERMIOS    
        path = config_get_config_filename();
        if ( access(path,R_OK|W_OK) != 0 ) {
            perror("access");
            free(path);
            path = null;
        }
    #else
        unsupported_configuration();
    #endif
    return path;
}

char *config_try_make_config_file() {
    char * path = config_get_config_file_safe();
    if ( path == null ) {
        if ( config_try_make_config_directory_one_dir(config_get_config_directory_top_internal()) ) {
            if ( config_try_make_config_directory_one_dir(config_get_config_directory_internal()) ) {
                final char * configFile = config_get_config_filename();
                if ( access(configFile,R_OK|W_OK) != 0 ) {
                    final FILE * file = fopen(configFile, "w");
                    if ( file == null ) {
                        free(configFile);
                    } else {
                        fclose(file);
                        path = configFile;
                    }   
                }
            }
        }
    }
    return path;
}

char *config_get_data_directory_safe() {
    char *path = null;
    #ifdef POSIX_TERMIOS
        char *rootPath = null;
        char * usr_share;
        int mode = R_OK|X_OK;
        asprintf(&usr_share,"/usr/share/%s/", APP_NAME);
        // Check if there is a local installation
        asprintf(&path, "%s/.local/share/%s/", getenv("HOME"), APP_NAME);
        if ( access(path,mode) == 0 ) {
            
        } else {
            free(path);
            if ( access(usr_share,mode) == 0 ) {
                path = strdup(usr_share);
            } else {
                path = null;
            }
        }
        free(usr_share);
    #else
        unsupported_configuration();
    #endif
    return path;
}

bool config_store() {
    bool res = false;
    #ifdef POSIX_TERMIOS
        final char * configPath = config_try_make_config_file();
        if ( configPath == null ) {
            log_msg("Cannot write config", LOG_WARNING);
        } else {
            final FILE * file = fopen(configPath, "w");
            if ( file == null ) {
                perror("fopen");
            } else {
                fprintf(file,"com.serial.baud_rate=%d\n", config.com.serial.baud_rate);
                if ( config.com.serial.port_name != null && strcmp("",config.com.serial.port_name)!=0) {
                    fprintf(file,"com.serial.port_name=%s\n", config.com.serial.port_name);
                }
                fprintf(file,"main.adaptater_detailled_settings_showned=%d\n", config.main.adaptater_detailled_settings_showned);
                fprintf(file,"commandLine.autoScrollEnabled=%d\n", config.commandLine.autoScrollEnabled);
                fprintf(file,"commandLine.showTimestamp=%d\n", config.commandLine.showTimestamp);
                fprintf(file,"vehicleExplorer.refreshRateS=%f\n", config.vehicleExplorer.refreshRateS);
                fclose(file);
            }
            free(configPath);
        }
    #else
        unsupported_configuration();
    #endif
    return res;
}

bool config_load() {
    bool res = false;
    #ifdef POSIX_TERMIOS
        char * configPath = config_get_config_file_safe();
        if ( configPath == null ) {
            log_msg("No configuration found", LOG_WARNING);
        } else {
            final FILE *file = fopen(configPath, "r");
            if ( file == null ) {
                perror("open");
            } else {
                char *line = null;
                size_t len = 0;
                ssize_t nread;
                char *eqChar;
                       
                while ((nread = getline(&line, &len, file)) != -1) {
                    if ( line[nread-1] == '\n' ) {
                        line[nread-1] = 0;
                    }
                    eqChar = strchr(line,'=');
                    if ( eqChar != null ) {
                        *eqChar = 0;
                        if ( strcmp(line,"com.serial.baud_rate") == 0 ) {
                            config.com.serial.baud_rate = atoi(eqChar + 1);
                        } else if ( strcmp(line,"com.serial.port_name") == 0 ) {
                            config.com.serial.port_name = strdup(eqChar + 1);
                        } else if ( strcmp(line,"main.adaptater_detailled_settings_showned") == 0 ) {
                            config.main.adaptater_detailled_settings_showned = atoi(eqChar+1);
                        } else if ( strcmp(line,"commandLine.autoScrollEnabled") == 0 ) {
                            config.commandLine.autoScrollEnabled = atoi(eqChar+1);
                        } else if ( strcmp(line,"commandLine.showTimestamp") == 0 ) {
                            config_commandLine_showTimestamp_set(atoi(eqChar+1));
                        } else if ( strcmp(line,"vehicleExplorer.refreshRateS") == 0 ) {
                            config.vehicleExplorer.refreshRateS = strtod(eqChar+1,null);
                        }
                    }
                    free(line);
                    line = null;
                    len = 0;
                }
                fclose(file);
            }
            free(configPath);
        }
    #else
        unsupported_configuration();
    #endif
    return res;
}

void config_onchange() {
    serial_list_set_selected_by_name(config.com.serial.port_name);
    final SERIAL port = serial_list_get_selected();
    if ( port == null ) {
        serial_list_selected = SERIAL_LIST_NO_SELECTED;
    } else {
        port->baud_rate = config.com.serial.baud_rate;
        config.ephemere.iface = obd_open_from_serial(port);
    }
}

