#include "libautodiag/sim/elm327/network.h"
#include "libautodiag/sim/elm327/elm327.h"

bool sim_elm327_network_is_connected(object_handle_t * h) {
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