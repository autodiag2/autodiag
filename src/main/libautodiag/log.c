#include "libautodiag/log.h"
#include "libautodiag/lib.h"

Logger logger = {
    .current_level = LOG_ERROR,
    .show_timestamp = false,
    .show_code_location = false,
    .use_colors = false
};

bool log_level_is_env_set() {
    return null != getenv("AUTODIAG_LOG_LEVEL");
}

void log_set_from_env() {
    char * logLevel = getenv("AUTODIAG_LOG_LEVEL");
    if ( logLevel == null ) {
        log_set_level(LOG_DEFAULT_LEVEL);
    } else {
        LogLevel level = log_level_from_str(logLevel);
        log_set_level(level);
    }
    char * logTimestamp = getenv("AUTODIAG_LOG_TIMESTAMP");
    if ( logTimestamp != null ) {
        if ( strcasecmp(logTimestamp, "false") == 0 || strcasecmp(logTimestamp, "0") == 0 ) {
            logger.show_timestamp = false;
        } else {
            logger.show_timestamp = true;
        }
    }
    char * codeLocation = getenv("AUTODIAG_LOG_CODE_LOCATION");
    if ( codeLocation != null ) {
        if ( strcasecmp(codeLocation, "false") == 0 || strcasecmp(codeLocation, "0") == 0 ) {
            logger.show_code_location = false;
        } else {
            logger.show_code_location = true;
        }
    }
    char * useColors = getenv("AUTODIAG_LOG_COLOR");
    if ( useColors != null ) {
        if ( strcasecmp(useColors, "false") == 0 || strcasecmp(useColors, "0") == 0 ) {
            logger.use_colors = false;
        } else {
            logger.use_colors = true;
        }
    }
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
    long milliseconds = tv.tv_usec / 1000;
    asprintf(&ctime, "%02d/%02d/%04d %02d:%02d:%02d.%03ld ", localtm->tm_mday, localtm->tm_mon, 1900 + localtm->tm_year, localtm->tm_hour, localtm->tm_min, localtm->tm_sec, milliseconds);
    return ctime;
}
void log_msg_internal(LogLevel level, char * file, int line, char *format, ...) {
    if ( level <= logger.current_level ) {

        va_list ap;

        va_start(ap, format);

        final char * header = strdup("");
        if ( logger.show_code_location ) {
            asprintf(&header, "%s:%d: ", file, line);
        }
        if ( logger.show_timestamp ) {
            char * tmpHeader;
            asprintf(&tmpHeader, "%s%s: ", header, log_get_current_time());
            free(header);
            header = tmpHeader;
        }

        char *colorSetter = strdup("");
        const char * colorCleaner = "\033[0m";
        if ( logger.use_colors ) {
            free(colorSetter);
            switch(level) {
                case LOG_DEBUG:
                    colorSetter = strdup("");
                    break;
                case LOG_INFO:
                    colorSetter = strdup("\033[34m");
                    break;
                case LOG_WARNING:
                    colorSetter = strdup("\033[33m");
                    break;
                case LOG_ERROR:
                    colorSetter = strdup("\033[31m");
                    break;
                case LOG_NONE:
                    colorSetter = strdup("");
                    break;
            }
        }
        char *formatMod;
        asprintf(&formatMod,"%s%s%s%s\n",header,colorSetter,format,colorCleaner);
        free(colorSetter);
        char *txt;
        if ( compat_vasprintf(&txt, formatMod, ap) == -1 ) {
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
void log_set_level(final LogLevel level) {
    logger.current_level = level;
}
bool log_has_level(final LogLevel level) {
    return level <= logger.current_level;
}
#include <stdio.h>

#if defined(OS_POSIX)
#   include <execinfo.h>
#   include <stdlib.h>

    void log_backtrace(void) {
        void *buffer[64];
        int nptrs = backtrace(buffer, 64);
        char **symbols = backtrace_symbols(buffer, nptrs);
        if (!symbols) return;

        printf("Backtrace (%d frames):\n", nptrs);
        for (int i = 0; i < nptrs; i++) {
            printf("%s\n", symbols[i]);
        }
        free(symbols);
    }

#elif defined(OS_WINDOWS)
#   include <windows.h>
#   include <dbghelp.h>

#   pragma comment(lib, "dbghelp.lib")

    void log_backtrace(void) {
        void *stack[64];
        unsigned short frames = CaptureStackBackTrace(0, 64, stack, NULL);

        SYMBOL_INFO *symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256, 1);
        if (!symbol) return;

        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);

        printf("Backtrace (%d frames):\n", frames);
        for (int i = 0; i < frames; i++) {
            if (SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol)) {
                printf("%i: %s - 0x%0llX\n", i, symbol->Name, symbol->Address);
            } else {
                printf("%i: ???\n", i);
            }
        }

        free(symbol);
        SymCleanup(process);
    }

#else
#   warning log_backtrace not implemented for this platform
    void log_backtrace(void) {}
#endif