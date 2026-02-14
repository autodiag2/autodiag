#include "libautodiag/sim/elm327/network.h"
#include "libautodiag/sim/elm327/elm327.h"

bool sim_elm327_network_is_connected(void * implPtr) {
    assert(implPtr != null);
    SimELM327Implementation * impl = (SimELM327Implementation*)implPtr;
    #ifdef OS_POSIX
        #ifdef OS_WINDOWS
            sock_t handle = impl->win_handle;
        #else        
            sock_t handle = impl->handle;
        #endif
        if ( handle == -1 ) {
            return false;
        }
    #elif defined OS_WINDOWS
        sock_t handle = impl->client_socket;
        if ( handle == INVALID_SOCKET ) {
            return false;
        }
    #else
    #   warning Unsupported OS
    #endif
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