#include "log.h"

Logger logger = {
    .current_level = LOG_ERROR,
    .show_timestamp = false
};

void log_set_from_env() {
    char * logLevel = getenv("AUTODIAG_LOG_LEVEL");
    if ( logLevel == null ) {
        log_set_level(LOG_ERROR);
    } else {
        log_set_level(log_level_from_str(logLevel));
    }
}

void log_set_level(LogLevel level) {
    logger.current_level = level;
}

bool log_has_level(final LogLevel level) {
    return logger.current_level <= level;
}

LogLevel log_get_level() {
    return logger.current_level;
}

LogLevel log_level_from_str(char *str) {
    if ( strcmp(str, "debug") == 0) {
        return LOG_DEBUG;
    } else if ( strcmp(str, "info") == 0) {
        return LOG_INFO;
    } else if ( strcmp(str, "warning") == 0) {
        return LOG_WARNING;
    } else if ( strcmp(str, "error") == 0) {
        return LOG_ERROR;
    } else if ( strcmp(str, "none") == 0) {
        return LOG_NONE;
    } else {
        fprintf(stderr, "Unknown log level given");
        exit(1);
    }
}

char *log_get_current_time() {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    struct tm * localtm = localtime(&(tv.tv_sec));
    char *ctime;
    asprintf(&ctime, "%d/%d/%d %d:%d:%d.%ld ", localtm->tm_mday, localtm->tm_mon, 1900 + localtm->tm_year, localtm->tm_hour, localtm->tm_min, localtm->tm_sec, tv.tv_usec);
    return ctime;
}

void log_msg(char *msg, LogLevel level) {
    if ( logger.current_level <= level ) {
        final char * header;
        if ( logger.show_timestamp ) {
            header = log_get_current_time();
        } else {
            header = strdup("");
        }
        char *ptr = msg, *init_ptr;
        int count;
        char msg_buffer[strlen(msg) + 1];
        while(*ptr != 0) {
            count = 0;
            while(*ptr != 0 && *ptr != '\n') {
                ptr ++;
                count ++;
            }
            memcpy(msg_buffer,ptr-count,count);
            msg_buffer[count] = 0;
            fprintf(stderr,"%s%s\n", header, msg_buffer);
            while(*ptr == '\n') ptr ++;
        }
        free(header);
    }
}

void module_debug(char * modname, char *msg) {
    const char * format = "%s: %s";
    const int formatting_caracters = 4;
    const int final_zero = 1;
    const int dest_size = strlen(msg) + strlen(modname) + strlen(format) - formatting_caracters + final_zero;
    char dest[dest_size];
    assert(1 <= dest_size);
    dest[dest_size-1] = 0;
    sprintf(dest, format, modname, msg);
    log_msg(dest, LOG_DEBUG);
}

void module_debug_init(char * modname) {
    module_debug(modname, "Module initialization");
}
