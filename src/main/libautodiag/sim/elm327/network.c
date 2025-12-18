#include "libautodiag/sim/elm327/network.h"

static int bind_any_available_port(sock_t server_fd, int start_port, int max_tries, int *out_port) {
    struct sockaddr_in addr;
    int opt = 1;

    #ifdef OS_WINDOWS
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
    #else
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #endif

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    for (int i = 0; i < max_tries; i++) {
        int port = start_port + i;
        addr.sin_port = htons(port);
        if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            *out_port = port;
            return 0;
        }
    }

    addr.sin_port = htons(0);
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
        socklen_t len = sizeof(addr);
        if (getsockname(server_fd, (struct sockaddr *)&addr, &len) == 0) {
            *out_port = ntohs(addr.sin_port);
            return 0;
        }
    }

    return -1;
}

int sim_elm327_network_start(int *bound_port) {
    #ifdef OS_WINDOWS
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return -1;
    #endif

    sock_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return -1;

    if (bind_any_available_port(server_fd, ELM327_NETWORK_PORT, 32, bound_port) < 0) {
        close_sock(server_fd);
        #ifdef OS_WINDOWS
            WSACleanup();
        #endif
        return -1;
    }

    if (listen(server_fd, ELM327_CONNECTION_BACKLOG) < 0) {
        close_sock(server_fd);
        #ifdef OS_WINDOWS
            WSACleanup();
        #endif
        return -1;
    }

    return (int)server_fd;
}

char * sim_elm327_network_location(struct sockaddr_in caddr) {

    char ip[50] = {0};
        inet_ntop(AF_INET, &caddr.sin_addr, ip, 49);

    return gprintf("%s:%d", ip, ntohs(caddr.sin_port));
}