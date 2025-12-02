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
#include "libautodiag/lang/lang.h"

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
    bool show_code_location;
} Logger;

extern Logger logger;

#define log_msg(level, msg, ...) \
    log_msg_internal(level, __FILE__, __LINE__, msg, ##__VA_ARGS__)

/**
 * @param level N
 */
void log_msg_internal(LogLevel level, char * file, int line, char *msg, ...);
void module_debug(char * modname, char *msg);
void module_debug_init(char * modname);
void log_set_level(final LogLevel level);
#define log_get_level() logger.current_level
bool log_has_level(final LogLevel level);
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
