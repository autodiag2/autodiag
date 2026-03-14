#ifndef __AD_LOG_H
#define __AD_LOG_H

#define _GNU_SOURCE
#include <stdio.h>
#include "libautodiag/string.h"
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdarg.h>
#include "libautodiag/lang/lang.h"
#include "libautodiag/compile_target.h"
#ifdef OS_ANDROID
#   include <android/log.h>
#endif

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
    bool use_colors;
} Logger;

extern Logger logger;

#ifdef COMPILE_RELEASE
#   define log_msg(level, msg, ...)
#else
#   define log_msg(level, msg, ...) log_msg_internal(level, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#endif
#define log_warn(msg, ...) log_msg(LOG_WARNING, msg, ##__VA_ARGS__)
#define log_info(msg, ...) log_msg(LOG_INFO, msg, ##__VA_ARGS__)
#define log_err(msg, ...) log_msg(LOG_ERROR, msg, ##__VA_ARGS__)
#define log_debug(msg, ...) log_msg(LOG_DEBUG, msg, ##__VA_ARGS__)

/**
 * @param level N
 */
void log_msg_internal(LogLevel level, char * file, int line, char *msg, ...);
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
bool log_level_is_env_set();
/**
 * Print a call stack
 */
void log_backtrace();
#endif
