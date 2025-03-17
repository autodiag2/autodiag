#include "sim/elm327/nvm.h"

bool elm327_sim_non_volatile_memory_parse(char * funcData, char *key, char *value) {
    ELM327emulation * elm327 = (ELM327emulation*)funcData;
    if ( strcasecmp(key,"nvm.protocol") == 0 ) {
        elm327->nvm.protocol = atoi(value);
        return true;
    }
    if ( strcasecmp(key,"nvm.protocolIsAuto") == 0 ) {
        elm327->nvm.protocol_is_auto = atoi(value);
        return true;
    }
    if ( strcasecmp(key,"nvm.user_memory") == 0 ) {
        elm327->nvm.user_memory = atoi(value);
        return true;
    }
    if ( strcasecmp(key,"nvm.programmable_parameters") == 0 ) {
        elm327->nvm.programmable_parameters = buffer_from_string(value);
        return true;
    } 
    if ( strcasecmp(key,"nvm.programmable_parameters_states") == 0 ) {
        elm327->nvm.programmable_parameters_states = buffer_from_string(value);
        return true;
    }    
    return false;
}
char * elm327_sim_non_volatile_get_filename() {
    #ifdef OS_WINDOWS
    #   include <windows.h>
        char tempPath[MAX_PATH];
        if ( ! GetTempPath(MAX_PATH, tempPath) ) {
            log_msg(LOG_ERROR, "failed to get the temp path");
        }
        char * path;
        asprintf(&path, "%s" PATH_FOLDER_DELIM "sim_memory.txt", tempPath);
        return path;
    #elif defined OS_POSIX
        return strdup("/tmp/sim_memory.txt");
    #else
    #   warning OS unsupported
    #endif
}
bool elm327_sim_non_volatile_wipe_out() {
    unlink(elm327_sim_non_volatile_get_filename());
    return true;
}
bool elm327_sim_non_volatile_memory_load(ELM327emulation * elm327, ELM327_SIM_INIT_TYPE type) {
    return parse_ini_file(elm327_sim_non_volatile_get_filename(), elm327_sim_non_volatile_memory_parse, elm327);
}

bool elm327_sim_non_volatile_memory_store(ELM327emulation * elm327) {
    if ( elm327->isMemoryEnabled ) {
        final FILE * file = fopen(elm327_sim_non_volatile_get_filename(), "w");
        if ( file == null ) {
            perror("fopen");
            return false;
        }
        fprintf(file,"nvm.user_memory=%d" FILE_EOL, elm327->nvm.user_memory);
        fprintf(file,"nvm.protocol=%d" FILE_EOL, elm327->nvm.protocol);
        fprintf(file,"nvm.protocolIsAuto=%d" FILE_EOL, elm327->nvm.protocol_is_auto);
        fprintf(file,"nvm.programmable_parameters=%s" FILE_EOL, buffer_to_string(elm327->nvm.programmable_parameters));
        fprintf(file,"nvm.programmable_parameters_states=%s" FILE_EOL, buffer_to_string(elm327->nvm.programmable_parameters_states));
        fclose(file);
        return true;
    } else {
        return false;
    }
}