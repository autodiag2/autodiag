#include "libautodiag/sim/sim.h"
#include "libautodiag/sim/elm327/elm327.h"
#include "libautodiag/sim/doip/doip.h"
#include "cJSON.h"

bool sim_network_is_connected(object_handle_t * h) {
    assert(h != null);
    #ifdef OS_POSIX
        sock_t handle = h->posix_handle;
    #elif defined OS_WINDOWS
        sock_t handle = h->win_socket;
    #else
    #   warning Unsupported OS
    #endif
    if ( handle == SOCK_T_INVALID ) {
        return false;
    }
    char buf;
    ssize_t ret = recv(handle, &buf, 1, MSG_PEEK);
    if (ret == 0) return false; // connection closed by peer
    #if defined OS_POSIX
    #   include <fcntl.h>
    #   include <errno.h>
        if (ret == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return 1; // still connected, no data
            return false; // error, consider disconnected
        }
    #elif defined OS_WINDOWS
        if (ret == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) return 1;
            return false;
        }
    #else
    #   warning Unsupported OS
    #endif
    return true; // data available, connection alive
}

SimECU * sim_search_ecu_by_address(Sim *sim, byte address) {
    for (int i = 0; i < sim->ecus->size; i++) {
        SimECU *ecu = sim->ecus->list[i];
        if ( address == ecu->address ) {
            return ecu;
        }
    }
    return null;
}
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
    assert(impl->handle != null);
    if ( sim_network_is_connected(impl->handle) ) {
        log_msg(LOG_DEBUG, "make a wait before sending the response to avoid write() before read() causing response loss");
        usleep(50e3);
    }
}
int sim_write(Sim * sim, int timeout_ms, byte * data, unsigned data_len) {
    assert(sim != null);
    SimImplementation * impl = sim->implementation;
    object_handle_t * client_handle = impl->handle;
    assert(!object_handle_t_invalid(client_handle));
    final int poll_result = object_handle_t_poll_write(client_handle, timeout_ms);
    if ( poll_result <= 0 ) {
        log_msg(LOG_WARNING, "timeout reached waiting for the other end");
        return -1;
    }
    int bytes_written = object_handle_t_write(client_handle, data, data_len);
    if ( bytes_written == -1 ) {
        perror("write");
    }
    return bytes_written;
}
int sim_read(Sim * sim, int timeout_ms, Buffer * readed) {
    assert(sim != null);
    assert(readed != null);
    buffer_ensure_capacity(readed, 500);
    SimImplementation * impl = sim->implementation;
    object_handle_t * client_handle = impl->handle;
    final int poll_result = object_handle_t_poll_read(client_handle, null, timeout_ms);
    if ( poll_result == -1 ) {
        log_msg(LOG_ERROR, "poll error: %s", strerror(errno));
        return -1;
    }
    int rv = object_handle_t_read(client_handle, readed->buffer, readed->size_allocated);
    if ( rv == -1 ) {
        log_msg(LOG_ERROR, "read error: %s", strerror(errno));
        return -1;
    }
    readed->size = rv;
    return rv;
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
