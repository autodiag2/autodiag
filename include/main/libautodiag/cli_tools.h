#ifndef __CLI_TOOLS_H
#define __CLI_TOOLS_H

#define argNext() i++;
#define argIs(value) ( i < argc && strcmp(argv[i],value) == 0 )
#define argForEach() for(int i = 1; i < argc; i++)
#define argCurrentIndex() i
#define argCurrent() ( i < argc ? argv[i] : null)

#define PRINT_MODULAR_H(sym) void sym(char *offset);

#define PRINT_MODULAR(sym, content) \
    void sym(char *offset) {\
    \
        char *format = strdup(content); \
    \
        char * format_upgraded = strdup("");\
        char * line = strsep(&format, "\n");\
        while(line != null) {\
            char *tmp;\
            asprintf(&tmp, "%s%s%s\n", format_upgraded, offset, line);\
            free(format_upgraded);\
            format_upgraded = tmp;\
            \
            line = strsep(&format, "\n");\
        }\
    \
        printf("%s", format_upgraded);\
    \
    }

#endif
