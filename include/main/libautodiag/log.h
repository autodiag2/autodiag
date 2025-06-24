#ifndef __LOG_H
#define __LOG_H

#define _GNU_SOURCE
#include <stdio.h>
#include "libautodiag/string.h"
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdarg.h>
#include "lang.h"

// do not change order, it is linked to options gui
typedef enum {
    LOG_NONE,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG
} LogLevel;

#define LOG_DEFAULT_LEVEL LOG_ERROR

typedef struct {
    LogLevel current_level;
    bool show_timestamp;
} Logger;

extern Logger logger;

/**
 * @param level N
 */
void log_msg(LogLevel level, char *msg, ...);
void module_debug(char * modname, char *msg);
void module_debug_init(char * modname);
#define log_set_level(level) logger.current_level = level;
#define log_get_level() logger.current_level
#define log_has_level(level) (level <= logger.current_level)
/**
 * Extract log level from given string
 */
LogLevel log_level_from_str(char *str);
char* log_level_to_str(LogLevel level);
/**
 * Get formatted time as string
 */
char *log_get_current_time();
void log_set_from_env();
bool log_is_env_set();
#endif
