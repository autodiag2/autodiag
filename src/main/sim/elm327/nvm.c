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

bool elm327_sim_non_volatile_memory_load(ELM327emulation * elm327) {
    return parse_ini_file(ELM327_SIM_NON_VOLATILE_MEMORY_PATH, elm327_sim_non_volatile_memory_parse, elm327);
}

bool elm327_sim_non_volatile_memory_store(ELM327emulation * elm327) {
    if ( elm327->isMemoryEnabled ) {
        final FILE * file = fopen(ELM327_SIM_NON_VOLATILE_MEMORY_PATH, "w");
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