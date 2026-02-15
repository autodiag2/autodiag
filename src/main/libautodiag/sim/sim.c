#include "libautodiag/sim/sim.h"
#include "libautodiag/sim/elm327/elm327.h"
#include "libautodiag/sim/doip/doip.h"
#include "cJSON.h"

void sim_init_with_defaults(Sim *sim) {
    sim->ecus = list_SimECU_new();
    final SimECU *ecu = sim_ecu_new(0xE8);
    list_SimECU_append(LIST_SIM_ECU(sim->ecus),ecu);
    sim->device_location = null;
}
bool sim_loop_daemon_wait_ready(bool * var) {
    assert(var != null);
    final int timeout_ms = 10000;
    final int step_ms = 20;
    final int step_n = timeout_ms / step_ms;
    int i = 0;
    for(; i < step_n && *var == false; i++) {
        usleep(step_ms * 1e3);
    }
    if ( i == step_n ) {
        log_msg(LOG_ERROR, "timeout while waiting for sim to be ready");
        return false;
    } else {
        usleep(step_ms * 1e3);
        return true;
    }
}
void sim_prevent_read_himself(Sim * sim) {
    assert(sim != null);
    SimImplementation * impl = sim->implementation;
    assert(impl != null);
    bool result = (impl->server_fd == -1);
    if ( result ) {
        log_msg(LOG_DEBUG, "make a wait before sending the response to avoid write() before read() causing response loss");
        usleep(50e3);
    }
}
int sim_write(Sim * sim, int timeout_ms, byte * data, unsigned int data_len) {
    assert(sim != null);
    #ifdef OS_POSIX
        if (sim->implementation->handle != -1) {
            SimImplementation * impl = sim->implementation;
            final int poll_result = file_pool_write_posix(impl->handle, timeout_ms);
            if ( poll_result <= 0 ) {
                log_msg(LOG_WARNING, "timeout reached waiting for the other end");
                return -1;
            }
            int bytes_written = write(impl->handle, data, data_len);
            if ( bytes_written == -1 ) {
                perror("write");
            }
            return bytes_written;
        }
    #endif
    #ifdef OS_WINDOWS
        if (impl->client_socket != INVALID_SOCKET) {
            int ret = send(impl->client_socket, (const char*)data, (int)data_len, 0);
            if (ret == SOCKET_ERROR) {
                log_msg(LOG_ERROR, "send failed: %d", WSAGetLastError());
                return -1;
            }
            return ret;
        }
    #endif
    if ( strcasecmp(sim->type,"elm327") == 0 ) {
        SimELM327 * elm327 = (SimELM327*)sim;
        SimELM327Implementation * impl = (SimELM327Implementation *)elm327->implementation;
        assert(data != null);
        #ifdef OS_WINDOWS
            assert(impl->win_handle != INVALID_HANDLE_VALUE);
            final int poll_result = file_pool_write(&impl->win_handle, timeout_ms);
            if ( poll_result <= 0 ) {
                log_msg(LOG_WARNING, "timeout reached waiting for the other end");
                return -1;
            }
            DWORD bytes_written = 0;
            if (!WriteFile(impl->win_handle, data, data_len, &bytes_written, null)) {
                log_msg(LOG_ERROR, "WriteFile failed with error %lu", GetLastError());
                return -1;
            }
            return bytes_written;
        #endif
    } else if ( strcasecmp(sim->type, "doip") == 0 ) {
        // nothing to do
    } else {
        log_msg(LOG_ERROR, "Device type '%s' not implemented", sim->type);
    }
    return -1;
}
int sim_read(Sim * sim, int timeout_ms, Buffer * readed) {
    assert(sim != null);
    assert(readed != null);
    buffer_ensure_capacity(readed, 500);
    #ifdef OS_POSIX
        if ( sim->implementation->handle != -1 ) {
            int res = file_pool_read_posix(sim->implementation->handle, null, timeout_ms);
            if ( res == -1 ) {
                log_msg(LOG_ERROR, "poll error: %s", strerror(errno));
                return -1;
            }
            int rv = read(sim->implementation->handle,readed->buffer,readed->size_allocated-1);
            if ( rv == -1 ) {
                log_msg(LOG_ERROR, "read error: %s", strerror(errno));
                return -1;
            }
            readed->size = rv;
            return rv;
        }
    #endif
    #ifdef OS_WINDOWS
        if (impl->client_socket != INVALID_SOCKET) {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(impl->client_socket, &rfds);

            struct timeval tv;
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;

            int sel = select(0, &rfds, null, null, &tv);
            if (sel <= 0) return -1;

            int ret = recv(impl->client_socket,
                        (char*)readed->buffer,
                        readed->size_allocated - 1,
                        0);

            if (ret <= 0) return -1;
            readed->size = ret;
            return ret;
        }
    #endif
    if ( strcasecmp(sim->type,"elm327") == 0 ) {
        SimELM327 * elm327 = (SimELM327*)sim;
        SimELM327Implementation * impl = (SimELM327Implementation*)elm327->implementation;        
        #ifdef OS_WINDOWS
            if ( ! ConnectNamedPipe(impl->win_handle, null) ) {
                DWORD err = GetLastError();
                if ( err == ERROR_PIPE_CONNECTED ) {
                    log_msg(LOG_DEBUG, "pipe already connected");
                } else {
                    if ( err == ERROR_NO_DATA ) {
                        log_msg(LOG_ERROR, "pipe closed");
                    } else {
                        log_msg(LOG_ERROR, "connexion au client échouée: (%lu)", GetLastError());
                    }
                    return -1;
                }
            }
            if ( file_pool_read(&impl->win_handle, null, timeout_ms) == -1 ) {
                log_msg(LOG_ERROR, "Error while pooling");
                return -1;
            }
            DWORD readedBytes = 0;
            final int success = ReadFile(impl->win_handle, readed->buffer, readed->size_allocated-1, &readedBytes, 0);
            if ( ! success ) {
                log_msg(LOG_ERROR, "read error : %lu ERROR_BROKEN_PIPE=%lu", GetLastError(), ERROR_BROKEN_PIPE);
                return -1;
            }
            if ( UINT_MAX < readedBytes ) {
                log_msg(LOG_ERROR, "Impossible has happend more bytes received than buffer->size=%u", readed->size);
                return -1;
            }
            readed->size = readedBytes;
            return readedBytes;
        #endif
    }
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
