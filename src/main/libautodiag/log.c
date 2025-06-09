#include "libautodiag/log.h"

Logger logger = {
    .current_level = LOG_ERROR,
    .show_timestamp = false
};

bool log_is_env_set() {
    return null != getenv("AUTODIAG_LOG_LEVEL");
}

void log_set_from_env() {
    char * logLevel = getenv("AUTODIAG_LOG_LEVEL");
    if ( logLevel == null ) {
        log_set_level(LOG_DEFAULT_LEVEL);
    } else {
        log_set_level(log_level_from_str(logLevel));
    }
}

void log_set_level(LogLevel level) {
    logger.current_level = level;
}

bool log_has_level(final LogLevel level) {
    return level <= logger.current_level;
}

LogLevel log_get_level() {
    return logger.current_level;
}

char* log_level_to_str(LogLevel level) {
    switch(level) {
        case LOG_DEBUG: return strdup("debug");
        case LOG_INFO: return strdup("info");
        case LOG_WARNING: return strdup("warning");        
        case LOG_ERROR: return strdup("error");
        case LOG_NONE: return strdup("none");
    }
    return null;
}

LogLevel log_level_from_str(char *str) {
    if ( strcasecmp(str, "debug") == 0) {
        return LOG_DEBUG;
    } else if ( strcasecmp(str, "info") == 0) {
        return LOG_INFO;
    } else if ( strcasecmp(str, "warning") == 0) {
        return LOG_WARNING;
    } else if ( strcasecmp(str, "error") == 0) {
        return LOG_ERROR;
    } else if ( strcasecmp(str, "none") == 0) {
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
    const time_t* time = (const time_t*)&(tv.tv_sec);
    struct tm * localtm = localtime(time);
    char *ctime;
    asprintf(&ctime, "%02d/%02d/%04d %02d:%02d:%02d.%03ld ", localtm->tm_mday, localtm->tm_mon, 1900 + localtm->tm_year, localtm->tm_hour, localtm->tm_min, localtm->tm_sec, tv.tv_usec/1000);
    return ctime;
}

void log_msg(LogLevel level, char *format, ...) {
    if ( level <= logger.current_level ) {

        va_list ap;

        va_start(ap, format);

        final char * header;
        if ( logger.show_timestamp ) {
            header = log_get_current_time();
        } else {
            header = strdup("");
        }

        char *formatMod;
        asprintf(&formatMod,"%s%s\n",header,format);
        char *txt;
        if ( vasprintf(&txt, formatMod, ap) == -1 ) {
            log_msg(LOG_ERROR, "Fill label impossible");
        } else {
            fprintf(stderr,"%s",txt);
            free(txt);
        }
        free(formatMod);
        free(header);

        va_end(ap);        
    }
}

void module_debug(char * modname, char *msg) {
    log_msg(LOG_DEBUG, "%s: %s", modname, msg);
}

void module_debug_init(char * modname) {
    module_debug(modname, "Module initialization");
}
