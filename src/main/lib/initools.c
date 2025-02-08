#include "lib/initools.h"

bool parse_ini_file_parse_a_line(final BUFFER buffer, bool func(char*,char*,char*), void *funcData) {
    char * eqChar = strchr(buffer->buffer,'=');
    if ( eqChar != null ) {
        *eqChar = 0;
        if ( func != null && ! func(funcData,buffer->buffer,eqChar+1) ) {
            return false;                    
        }
    }
    return true;
}

bool parse_ini_file(char *iniFile, bool func(char*,char*,char*), void *funcData) {
    FILE_READ_LINES_WITH(iniFile,parse_ini_file_parse_a_line,func,funcData);
}


