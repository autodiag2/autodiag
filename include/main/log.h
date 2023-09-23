#ifndef __LOG_H
#define __LOG_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include "globals.h"

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_NONE
} LogLevel;

typedef struct {
    LogLevel current_level;
    bool show_timestamp;
} Logger;

extern Logger logger;

/**
 * @param level N
 */
void log_msg(char *msg, LogLevel level);
void module_debug(char * modname, char *msg);
void module_debug_init(char * modname);
void log_set_level(LogLevel level);
LogLevel log_get_level();
bool log_has_level(final LogLevel level);
/**
 * Extract log level from given string
 */
LogLevel log_level_from_str(char *str);
/**
 * Get formatted time as string
 */
char *log_get_current_time();
void log_set_from_env();
#endif
