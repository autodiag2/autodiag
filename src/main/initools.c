#include "initools.h"

bool parse_ini_file(char *iniFile, bool func(char*,char*,char*), void *funcData) {
    FILE * file = fopen(iniFile, "r");
    if ( file == null ) {
        perror("fopen");
        return false;
    } else {
        char *line = null;
        size_t len = 0;
        ssize_t nread;
        char *eqChar;
                   
        while ((nread = getline(&line, &len, file)) != -1) {
            if ( line[nread-1] == '\n' ) {
                line[nread-1] = 0;
            }
            eqChar = strchr(line,'=');
            if ( eqChar == null ) {
                free(line);
            } else {
                *eqChar = 0;
                if ( func(funcData,line,eqChar+1) ) {
                    
                } else {
                    free(line);
                }
            }
            line = null;
            len = 0;
        }
        fclose(file);
        return true;
    }
}
