#include "libautodiag/sim/elm327/nvm.h"

typedef struct {
    SimELM327 * elm327;
    int load_mask;
} SIM_ELM327_NVM_LOAD_DATA;

bool sim_elm327_non_volatile_memory_parse(char * funcData, char *key, char *value) {
    SIM_ELM327_NVM_LOAD_DATA * data = (SIM_ELM327_NVM_LOAD_DATA*)funcData;
    SimELM327 * elm327 = (SimELM327*)data->elm327;
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
        final Buffer * pp_pending = buffer_from_ascii_hex(value);
        if ( pp_pending == null || elm327->nvm.programmable_parameters->size < pp_pending->size ) {
            log_msg(LOG_WARNING, "Potential memory corruption detected - NVM dropped !");
            log_msg(LOG_DEBUG, "reading: %s", value);
        } else {
            elm327->nvm.programmable_parameters_pending = pp_pending;
            for(int i = 0; i < elm327->nvm.programmable_parameters_pending->size; i++) {
                if ( (data->load_mask & elm327->programmable_parameters_pending_load_type->buffer[i]) != 0 ) {
                    elm327->nvm.programmable_parameters->buffer[i] = elm327->nvm.programmable_parameters_pending->buffer[i];
                }
            }
        }
        return true;
    } 
    if ( strcasecmp(key,"nvm.programmable_parameters_states") == 0 ) {
        final Buffer * pp_states = buffer_from_ascii_hex(value);
        if ( pp_states == null || elm327->nvm.programmable_parameters_states->size < pp_states->size ) {
            log_msg(LOG_WARNING, "Potential memory corruption detected - NVM dropped !");
            log_msg(LOG_DEBUG, "reading: %s", value);
        } else {
            elm327->nvm.programmable_parameters_states = pp_states;
        }
        return true;
    }    
    return false;
}
char * sim_elm327_non_volatile_get_filename() {
    #ifdef OS_WINDOWS
    #   include "libautodiag/windows.h"
        char tempPath[MAX_PATH];
        if ( ! GetTempPath(MAX_PATH, tempPath) ) {
            log_msg(LOG_ERROR, "failed to get the temp path");
        }
        char * path;
        asprintf(&path, "%s" PATH_FOLDER_DELIM "sim_memory.txt", tempPath);
        return path;
    #elif defined OS_ANDROID
        #include "libautodiag/jni/tmp_file.h"
        char *path;
        asprintf(&path, "%s/sim_memory.txt", jni_tmp_dir_get());
        return path;
    #elif defined OS_POSIX
        return strdup("/tmp/sim_memory.txt");
    #else
    #   warning OS unsupported
    #endif
}
bool sim_elm327_non_volatile_wipe_out() {
    unlink(sim_elm327_non_volatile_get_filename());
    return true;
}
bool sim_elm327_non_volatile_memory_load(SimELM327 * elm327, int load_mask) {
    SIM_ELM327_NVM_LOAD_DATA data = { 
        .elm327 = elm327, 
        .load_mask = load_mask
    };
    return parse_ini_file(sim_elm327_non_volatile_get_filename(), sim_elm327_non_volatile_memory_parse, &data);
}

bool sim_elm327_non_volatile_memory_store(SimELM327 * elm327) {
    if ( elm327->isMemoryEnabled ) {
        final FILE * file = fopen(sim_elm327_non_volatile_get_filename(), "w");
        if ( file == null ) {
            perror("fopen");
            return false;
        }
        fprintf(file,"nvm.user_memory=%d" FILE_EOL, elm327->nvm.user_memory);
        fprintf(file,"nvm.protocol=%d" FILE_EOL, elm327->nvm.protocol);
        fprintf(file,"nvm.protocolIsAuto=%d" FILE_EOL, elm327->nvm.protocol_is_auto);
        fprintf(file,"nvm.programmable_parameters=%s" FILE_EOL, buffer_to_hex_string(elm327->nvm.programmable_parameters_pending));
        fprintf(file,"nvm.programmable_parameters_states=%s" FILE_EOL, buffer_to_hex_string(elm327->nvm.programmable_parameters_states));
        fclose(file);
        return true;
    } else {
        return false;
    }
}