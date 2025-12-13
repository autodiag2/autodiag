#include "libautodiag/sim/sim.h"


int sim_write(void *handle, int timeout_ms, byte * data, unsigned int data_len) {
    assert(handle != null);
    assert(data != null);
    final int poll_result = file_pool_write(handle, timeout_ms);
    if ( poll_result <= 0 ) {
        log_msg(LOG_WARNING, "timeout reached waiting for the other end");
        return -1;
    }
    #if defined OS_WINDOWS
        DWORD bytes_written = 0;
        if (!WriteFile(*((HANDLE*)handle), data, data_len, &bytes_written, null)) {
            log_msg(LOG_ERROR, "WriteFile failed with error %lu", GetLastError());
            return -1;
        }
        return bytes_written;
    #elif defined OS_POSIX
        int bytes_written = write(*((int*)handle), data, data_len);
        if ( bytes_written == -1 ) {
            perror("write");
        }
        return bytes_written;
    #else
    #   warning OS unsupported
    #endif
    return -1;
}
int sim_read(void *handle, int timeout_ms, Buffer * readed) {
    assert(handle != null);
    assert(readed != null);
    buffer_ensure_capacity(readed, 500);
    #ifdef OS_WINDOWS
        if ( ! ConnectNamedPipe(*handle, null) ) {
            DWORD err = GetLastError();
            if ( err == ERROR_PIPE_CONNECTED ) {
                log_msg(LOG_DEBUG, "pipe already connected");
            } else {
                if ( err == ERROR_NO_DATA ) {
                    log_msg(LOG_ERROR, "pipe closed");
                } else {
                    log_msg(LOG_ERROR, "connexion au client échouée: (%lu)", GetLastError());
                }
                return false;
            }
        }
        if ( file_pool_read(handle, null, timeout_ms) == -1 ) {
            log_msg(LOG_ERROR, "Error while pooling");
            return -1;
        }
        DWORD readedBytes = 0;
        final int success = ReadFile(*((HANDLE*)handle), readed->buffer, readed->size_allocated-1, &readedBytes, 0);
        if ( ! success ) {
            log_msg(LOG_ERROR, "read error : %lu ERROR_BROKEN_PIPE=%lu", GetLastError(), ERROR_BROKEN_PIPE);
            return -1;
        }
        if ( UINT_MAX < readedBytes ) {
            log_msg(LOG_ERROR, "Impossible has happend more bytes received than buffer->size=%u", buffer->size);
            return -1;
        }
        readed->size = readedBytes;
        return readedBytes;
    #elif defined OS_POSIX
        int res = file_pool_read(handle, null, timeout_ms);
        if ( res == -1 ) {
            log_msg(LOG_ERROR, "poll error: %s", strerror(errno));
            return -1;
        }
        int rv = read(*((int*)handle),readed->buffer,readed->size_allocated-1);
        if ( rv == -1 ) {
            log_msg(LOG_ERROR, "read error: %s", strerror(errno));
            return -1;
        }
        readed->size = rv;
        return rv;
    #else
    #   warning OS unsupported
    #endif
    return -1;
}

int sim_load_from_json(Sim * sim, char * json_context) {
    cJSON * root = null;
    FILE *f = fopen(json_context, "r");
    if ( f ) {
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *buf = (char *)malloc(len + 1);
        fread(buf, 1, len, f);
        buf[len] = 0;
        fclose(f);
        root = cJSON_Parse(buf);
        free(buf);
    } else {
        if (errno == ENOENT || errno == ENAMETOOLONG) {
            root = cJSON_Parse(json_context);
            if ( ! root ) {
                log_msg(LOG_ERROR, "Context '%s' is not a file and not a json string");
            }
        } else if (errno == EACCES) {
            log_msg(LOG_ERROR, "File '%s' exists but missing reading permission", json_context);
        }
    }
    if (!root) {
        log_msg(LOG_ERROR, "Impossible to get the flow from the json aborting ...");
        return GENERIC_FUNCTION_ERROR;
    }
    list_SimECU_clear(LIST_SIM_ECU(sim->ecus));
    if ( ! cJSON_IsArray(root) ) {
        assert(cJSON_IsObject(root));
        cJSON * arr = cJSON_CreateArray();
        cJSON_AddItemToArray(arr, root);
        root = arr;
    }
    if ( 0 == cJSON_GetArraySize(root) ) {
        log_msg(LOG_ERROR, "No ECUs in this file");
        return GENERIC_FUNCTION_ERROR;
    }
    for(int i = 0; i < cJSON_GetArraySize(root); i++) {
        cJSON * obj = cJSON_GetArrayItem(root, i);
        char *context = cJSON_PrintUnformatted(obj);
        cJSON * address_obj = cJSON_GetObjectItem(obj, "ecu");
        char * address_str = address_obj->valuestring;
        final Buffer * address = buffer_from_ascii_hex(address_str);
        final SimECU * ecu = sim_ecu_new(address->buffer[address->size-1]);
        ecu->generator = sim_ecu_generator_new_replay();
        ecu->generator->context = context;
        list_SimECU_append(LIST_SIM_ECU(sim->ecus), ecu);
    }
    return GENERIC_FUNCTION_SUCCESS;
}
