#include "libprog/config.h"

#define _DEFAULT_CONFIG { \
    .com = { \
        .connectAtStartup = true, \
        .serial = { \
            .baud_rate = SERIAL_DEFAULT_BAUD_RATE, \
            .device_location = null \
        } \
    }, \
    .main = { \
        .adaptater_detailled_settings_showned = false \
    }, \
    .commandLine = { \
        .autoScrollEnabled = true, \
        .showTimestamp = false \
    }, \
    .recorder = { \
        .enabled = false, \
        .filepath = null \
    }, \
    .ephemere = { \
        .iface = null, \
        .device_table = null \
    }, \
    .vehicleExplorer = { \
        .refreshRateS = 0.1, \
        .autoRefresh = true \
    }, \
    .log = { \
        .level = LOG_DEFAULT_LEVEL \
    }, \
    .vehicleInfos = { \
        .vin = null \
    } \
};
const Config DEFAULT_CONFIG = _DEFAULT_CONFIG; 
Config config = _DEFAULT_CONFIG;

void config_commandLine_showTimestamp_set(final bool state) {
    config_initiated_check();
    config.commandLine.showTimestamp = state;
    logger.show_timestamp = state;
}

void config_dump(Config * c) {
    config_initiated_check();
    log_msg(LOG_DEBUG, "Config {");
    log_msg(LOG_DEBUG,"    com.serial {");
    log_msg(LOG_DEBUG,"        baud_rate=%d, device_location=%s", c->com.serial.baud_rate, c->com.serial.device_location);
    log_msg(LOG_DEBUG,"    }");
    log_msg(LOG_DEBUG,"    main.adaptater_detailled_settings_showned=%d",c->main.adaptater_detailled_settings_showned);
    log_msg(LOG_DEBUG,"    commandLine {");
    log_msg(LOG_DEBUG,"        autoScrollEnabled=%d, showTimestamp=%d", c->commandLine.autoScrollEnabled, c->commandLine.showTimestamp);
    log_msg(LOG_DEBUG,"    }");
    log_msg(LOG_DEBUG,"}");
}

char *config_get_config_filename() {
    config_initiated_check();
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
    config_initiated_check();
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
    config_initiated_check();
    final char * configFile = config_get_config_filename();
    if ( mkdir_p(configFile) ) {
        #if defined OS_POSIX                
            if ( access(configFile,R_OK|W_OK) != 0 ) {
                if ( chmod(configFile,0x600) != 0 ) {
                    final FILE * file = fopen(configFile, "w");
                    if ( file == null ) {
                        perror("config_try_make_config_file: fopen");
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
    config_initiated_check();
    config = DEFAULT_CONFIG;
    config_init();
    return config_store();
}

bool config_store() {
    config_initiated_check();
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
            if ( config.com.serial.device_location != null && strcmp("",config.com.serial.device_location)!=0) {
                fprintf(file,"com.serial.device_location=%s" FILE_EOL, config.com.serial.device_location);
            }
            fprintf(file, "com.connectAtStartup=%d" FILE_EOL, config.com.connectAtStartup);
            fprintf(file,"main.adaptater_detailled_settings_showned=%d" FILE_EOL, config.main.adaptater_detailled_settings_showned);
            fprintf(file,"commandLine.autoScrollEnabled=%d" FILE_EOL, config.commandLine.autoScrollEnabled);
            fprintf(file,"commandLine.showTimestamp=%d" FILE_EOL, config.commandLine.showTimestamp);
            fprintf(file,"recorder.enabled=%d" FILE_EOL, config.recorder.enabled);
            fprintf(file,"recorder.filepath=%s" FILE_EOL, config.recorder.filepath);
            fprintf(file,"vehicleExplorer.refreshRateS=%f" FILE_EOL, config.vehicleExplorer.refreshRateS);
            fprintf(file,"vehicleExplorer.autoRefresh=%d" FILE_EOL, config.vehicleExplorer.autoRefresh);
            if ( config.vehicleInfos.vin != null && strcmp("",config.vehicleInfos.vin)!=0) {
                fprintf(file,"vehicleInfos.vin=%s" FILE_EOL, config.vehicleInfos.vin);
            }
            fprintf(file,"log.level=%d" FILE_EOL, config.log.level);
            fclose(file);
            res = true;
        }
        free(configPath);
    }
    return res;
}
static double config_strtod(char * value) {
    char *tmp = strdup(value);
    for (char *p = tmp; *p; p++)
        if (*p == ',') *p = '.';

    double d = strtod(tmp, NULL);
    free(tmp);
    return d;
}
static bool config_load_parser(char * funcData, char *key, char *value) {
    config_initiated_check();
    if ( strcasecmp(key,"com.serial.baud_rate") == 0 ) {
        config.com.serial.baud_rate = atoi(value);
        return true;
    } else if ( strcasecmp(key,"com.connectAtStartup") == 0 ) {
        config.com.connectAtStartup = atoi(value);
        return true;
    } else if ( strcasecmp(key,"com.serial.device_location") == 0 ) {
        config.com.serial.device_location = strdup(value);
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
    } else if ( strcasecmp(key,"recorder.enabled") == 0 ) {
        config.recorder.enabled = atoi(value);
        return true;
    } else if ( strcasecmp(key, "recorder.filepath") == 0 ) {
        config.recorder.filepath = strdup(value);
        return true;
    } else if ( strcasecmp(key,"vehicleExplorer.refreshRateS") == 0 ) {
        config.vehicleExplorer.refreshRateS = config_strtod(value);
        return true;
    } else if ( strcasecmp(key,"vehicleExplorer.autoRefresh") == 0 ) {
        config.vehicleExplorer.autoRefresh = config_strtod(value);        
        return true;
    } else if ( strcasecmp(key, "log.level") == 0 ) {
        config.log.level = config_strtod(value);        
        return true;
    } else if ( strcasecmp(key, "vehicleInfos.vin") == 0 ) {
        config.vehicleInfos.vin = strdup(value);
        return true;
    }
    return false;
}

void config_init() {
    if ( config.ephemere.iface == null ) {
        config.ephemere.iface = viface_new();
    }
    if ( config.ephemere.device_table != null ) {
        object_SerialTable_free(config.ephemere.device_table);
    }
    config.ephemere.device_table = object_SerialTable_new();
}
bool config_load() {
    config_initiated_check();
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
    viface_close(config.ephemere.iface);
    config_initiated_check();
    if ( config.com.serial.device_location != null ) {
        serial_table_add_if_not_in_by_location(config.ephemere.device_table, config.com.serial.device_location);
        serial_table_set_selected_by_location(config.ephemere.device_table, config.com.serial.device_location);
    }
    final Serial * port = serial_table_get_selected(config.ephemere.device_table);
    if ( port == null ) {
        config.ephemere.device_table->selected_index = SERIAL_TABLE_NO_SELECTED;
    } else {
        port->baud_rate = config.com.serial.baud_rate;
        viface_recorder_reset(config.ephemere.iface);
        viface_recorder_set_state(config.ephemere.iface, config.recorder.enabled);
        if ( viface_open_from_iface_device(config.ephemere.iface, AD_DEVICE(port))) {
            if ( serial_table_update_device(config.ephemere.device_table, AD_DEVICE(port), config.ephemere.iface->device) ) {
                log_msg(LOG_WARNING, "Device update in the table of devices has failed, continuing ...");
            }
            if ( config.vehicleInfos.vin != null && 17 <= strlen(config.vehicleInfos.vin) ) {
                config.ephemere.iface->vehicle->vin = buffer_from_ascii(config.vehicleInfos.vin);
                viface_fill_infos_from_vin(config.ephemere.iface);
            }
        }
    }
}