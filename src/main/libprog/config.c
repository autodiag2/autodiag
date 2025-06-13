#include "libprog/config.h"

#define _DEFAULT_CONFIG { \
    .com = { \
        .serial = { \
            .baud_rate = SERIAL_DEFAULT_BAUD_RATE, \
            .port_name = null \
        } \
    }, \
    .main = { \
        .adaptater_detailled_settings_showned = false \
    }, \
    .commandLine = { \
        .autoScrollEnabled = true, \
        .showTimestamp = false \
    }, \
    .ephemere = { \
        .iface = null \
    }, \
    .vehicleExplorer = { \
        .refreshRateS = 0.1, \
        .autoRefresh = true \
    }, \
    .log = { \
        .level = LOG_DEFAULT_LEVEL \
    } \
};
const Config DEFAULT_CONFIG = _DEFAULT_CONFIG; 
Config config = _DEFAULT_CONFIG;

void config_commandLine_showTimestamp_set(final bool state) {
    config.commandLine.showTimestamp = state;
    logger.show_timestamp = state;
}

void config_dump(Config * config) {
    log_msg(LOG_DEBUG, "Config {");
    log_msg(LOG_DEBUG,"    com.serial {");
    log_msg(LOG_DEBUG,"        baud_rate=%d, port_name=%s", config->com.serial.baud_rate, config->com.serial.port_name);
    log_msg(LOG_DEBUG,"    }");
    log_msg(LOG_DEBUG,"    main.adaptater_detailled_settings_showned=%d",config->main.adaptater_detailled_settings_showned);
    log_msg(LOG_DEBUG,"    commandLine {");
    log_msg(LOG_DEBUG,"        autoScrollEnabled=%d, showTimestamp=%d", config->commandLine.autoScrollEnabled, config->commandLine.showTimestamp);
    log_msg(LOG_DEBUG,"    }");
    log_msg(LOG_DEBUG,"}");
}

char *config_get_config_filename() {
    char *path;
    #if defined OS_WINDOWS
        // Windows application data directory
        asprintf(&path,"%s\\" APP_NAME "\\" CONFIG_DEFAULT_FILENAME, getenv("APPDATA"));    
    #elif defined OS_UNIX
        // XDG Base Directory Specification
        asprintf(&path,"%s/.config/" APP_NAME "/" CONFIG_DEFAULT_FILENAME, getenv("HOME"));
    #else
    #   warning OS not supported
    #endif
    return path;
}

/**
 * Get the config file
 */
char *config_get_config_file_safe() {
    char *configFile = config_get_config_filename();
    #if defined OS_POSIX || defined OS_WINDOWS
        if ( access(configFile,R_OK|W_OK) != 0 ) {
            if ( chmod(configFile,0x600) != 0 ) {
                free(configFile);
                configFile = null;
            }
        }
    #else
    #   warning Unsupported OS        
    #endif
    return configFile;
}

char *config_try_make_config_file() {
    final char * configFile = config_get_config_filename();
    if ( mkdir_p(configFile) ) {
        #if defined OS_POSIX                
            if ( access(configFile,R_OK|W_OK) != 0 ) {
                if ( chmod(configFile,0x600) != 0 ) {
                    final FILE * file = fopen(configFile, "w");
                    if ( file == null ) {
                        perror("fopen");
                        log_msg(LOG_ERROR, "Cannot write the config '%s'", configFile);
                    } else {
                        fclose(file);
                    }   
                }
            }
        #else
        #   warning Unsupported OS        
        #endif
    } else {
        log_msg(LOG_ERROR, "error during mkdir_p");
    }
    return configFile;
}

bool config_reset() {
    config = DEFAULT_CONFIG;
    return config_store();
}

bool config_store() {
    bool res = false;
    final char * configPath = config_try_make_config_file();
    if ( configPath == null ) {
        log_msg(LOG_WARNING, "Cannot write config %s", configPath);
    } else {
        final FILE * file = fopen(configPath, "w");
        if ( file == null ) {
            log_msg(LOG_ERROR, "Config path is not built %s", configPath);
        } else {
            fprintf(file,"com.serial.baud_rate=%d" FILE_EOL, config.com.serial.baud_rate);
            if ( config.com.serial.port_name != null && strcmp("",config.com.serial.port_name)!=0) {
                fprintf(file,"com.serial.port_name=%s" FILE_EOL, config.com.serial.port_name);
            }
            fprintf(file,"main.adaptater_detailled_settings_showned=%d" FILE_EOL, config.main.adaptater_detailled_settings_showned);
            fprintf(file,"commandLine.autoScrollEnabled=%d" FILE_EOL, config.commandLine.autoScrollEnabled);
            fprintf(file,"commandLine.showTimestamp=%d" FILE_EOL, config.commandLine.showTimestamp);
            fprintf(file,"vehicleExplorer.refreshRateS=%f" FILE_EOL, config.vehicleExplorer.refreshRateS);
            fprintf(file,"vehicleExplorer.autoRefresh=%d" FILE_EOL, config.vehicleExplorer.autoRefresh);
            fprintf(file,"log.level=%d" FILE_EOL, config.log.level);
            fclose(file);
            res = true;
        }
        free(configPath);
    }
    return res;
}

bool config_load_parser(char * funcData, char *key, char *value) {
    if ( strcasecmp(key,"com.serial.baud_rate") == 0 ) {
        config.com.serial.baud_rate = atoi(value);
        return true;
    } else if ( strcasecmp(key,"com.serial.port_name") == 0 ) {
        config.com.serial.port_name = strdup(value);
        return true;
    } else if ( strcasecmp(key,"main.adaptater_detailled_settings_showned") == 0 ) {
        config.main.adaptater_detailled_settings_showned = atoi(value);
        return true;
    } else if ( strcasecmp(key,"commandLine.autoScrollEnabled") == 0 ) {
        config.commandLine.autoScrollEnabled = atoi(value);
        return true;
    } else if ( strcasecmp(key,"commandLine.showTimestamp") == 0 ) {
        config_commandLine_showTimestamp_set(atoi(value));
        return true;
    } else if ( strcasecmp(key,"vehicleExplorer.refreshRateS") == 0 ) {
        config.vehicleExplorer.refreshRateS = strtod(value,null);
        return true;
    } else if ( strcasecmp(key,"vehicleExplorer.autoRefresh") == 0 ) {
        config.vehicleExplorer.autoRefresh = strtod(value,null);        
        return true;
    } else if ( strcasecmp(key,"log.level") == 0 ) {
        config.log.level = strtod(value,null);        
        return true;
    }
    return false;
}

bool config_load() {
    bool res = false;
    char * configPath = config_get_config_file_safe();
    if ( configPath == null ) {
        log_msg(LOG_WARNING, "No configuration found");
    } else {
        res = parse_ini_file(configPath, config_load_parser, null);
        free(configPath);
    }
    return res;
}
void config_onchange() {
    serial_list_set_selected_by_name(config.com.serial.port_name);
    final SERIAL port = serial_list_get_selected();
    if ( port == null ) {
        serial_list_selected = SERIAL_LIST_NO_SELECTED;
    } else {
        port->baud_rate = config.com.serial.baud_rate;
        config.ephemere.iface = obd_open_from_device(port);
    }
}

