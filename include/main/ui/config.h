#ifndef __CONFIG_H
#define __CONFIG_H

#include "globals.h"
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "com/serial/obd/obd.h"
#include "ui/config.h"

/**
 * Define general configuration of the gui, values can be modified by the options gui or directly in file
 */
typedef struct {
    struct {
        struct {
            // port speed codified as in options screen
            int baud_rate;
            // port name as showned in options screen
            char *port_name;
        } serial;
    } com;
    struct {
        bool adaptater_detailled_settings_showned;
    } main;
    struct {
        bool autoScrollEnabled;
        bool showTimestamp;
    } commandLine;
    struct {
        OBDIFACE iface;
    } ephemere;
    struct {
        double refreshRateS;
    } vehicleExplorer;
} Config;

#define CONFIG_DEFAULT_FILENAME "config.ini"

extern Config config;

/**
 * Store config to filesystem
 */
bool config_store();
/**
 * Load config from filesystem
 */
bool config_load();
/**
 * Apply changes to the system
 */
void config_onchange();
/**
 * Get the storage directory
 */
char * config_get_data_directory_safe();
void config_dump(Config * config);
void config_commandLine_showTimestamp_set(final bool state);

#endif
