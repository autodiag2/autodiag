#include "libautodiag/com/network.h"

char * network_location(struct sockaddr_in caddr) {
    char ip[50] = {0};
        inet_ntop(AF_INET, &caddr.sin_addr, ip, 49);

    return gprintf("%s:%d", ip, ntohs(caddr.sin_port));
}

struct sockaddr_in network_location_to_object(char * location) {
    struct sockaddr_in addr;
    char host[500];
    char port_str[8] = "35000";
    const char *colon = strchr(location, ':');
    if (colon) {
        size_t len = colon - location;
        if (len >= sizeof(host)) len = sizeof(host) - 1;
        memcpy(host, location, len);
        host[len] = 0;
        strncpy(port_str, colon + 1, sizeof(port_str) - 1);
    } else {
        strncpy(host, location, sizeof(host) - 1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port_str));

    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        log_msg(LOG_ERROR, "invalid address: %s", host);
        memset(&addr, 0, sizeof(addr));
    }
    return addr;
}

static int bind_any_available_port(sock_t server_fd, int start_port, int max_tries, int *out_port) {
    struct sockaddr_in addr;
    int opt = 1;

    #ifdef OS_POSIX
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #elif defined OS_WINDOWS
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
    #else
    #   warning OS Unsupported
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
int sock_t_get_port(sock_t s) {
    
    if ( s == SOCK_T_INVALID ) return -1;

    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (getsockname(s, (struct sockaddr *)&addr, &len) < 0) {
        return -1;
    }

    if (addr.ss_family == AF_INET) {
        struct sockaddr_in *a = (struct sockaddr_in *)&addr;
        return ntohs(a->sin_port);
    }

    if (addr.ss_family == AF_INET6) {
        struct sockaddr_in6 *a = (struct sockaddr_in6 *)&addr;
        return ntohs(a->sin6_port);
    }

    return -1;
}
sock_t network_udp_start(int *bound_port, int start_port) {
    #if defined(OS_WINDOWS) && ! defined(OS_POSIX)
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return SOCK_T_INVALID;
    #endif

    sock_t server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) return SOCK_T_INVALID;

    if (bind_any_available_port(server_fd, start_port, 32, bound_port) < 0) {
        close_sock(server_fd);
        #if defined(OS_WINDOWS) && ! defined(OS_POSIX)
            WSACleanup();
        #endif
        return SOCK_T_INVALID;
    }

    return server_fd;
}

int network_udp_enable_broadcast(sock_t s) {
    int yes = 1;
    #ifdef OS_POSIX
        return setsockopt(s, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));
    #elif defined OS_WINDOWS
        return setsockopt(s, SOL_SOCKET, SO_BROADCAST, (const char *)&yes, sizeof(yes));
    #else
    #   warning Unsupported OS
    #endif
    return -1;
}

int network_udp_set_reuseaddr(sock_t s) {
    int yes = 1;
    #ifdef OS_POSIX
        return setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    #elif defined OS_WINDOWS
        return setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
    #else
    #   warning Unsupported OS
    #endif
    return -1;
}
bool network_is_connected(sock_t s) {

    if (s == SOCK_T_INVALID)
        return false;

    int so_type;
    #ifdef OS_POSIX
        socklen_t len = sizeof(so_type);
        if (getsockopt(s, SOL_SOCKET, SO_TYPE, &so_type, &len) == -1) {
            if (errno == ENOTSOCK)
                return false;
            return false;
        }
    #elif defined OS_WINDOWS
        int len = sizeof(so_type);

        if (getsockopt(s, SOL_SOCKET, SO_TYPE, (char *)&so_type, &len) == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAENOTSOCK)
                return false;
            return false;
        }
    #else
    #   warning unsupported OS
        return false;
    #endif

    switch (so_type) {
        case SOCK_STREAM: {
            fd_set rfds;
            fd_set efds;

            FD_ZERO(&rfds);
            FD_ZERO(&efds);

            FD_SET(s, &rfds);
            FD_SET(s, &efds);

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 0;

            int sel = select((int)(s + 1), &rfds, NULL, &efds, &tv);
            if (sel < 0)
                return false;

            if (FD_ISSET(s, &efds))
                return false;

            if (FD_ISSET(s, &rfds)) {
                char buf;
                #ifdef OS_POSIX
                    ssize_t ret = recv(s, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
                    if (ret == 0)
                        return false;
                    if (ret < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            return true;
                        return false;
                    }
                #elif defined OS_WINDOWS
                    int ret = recv(s, &buf, 1, MSG_PEEK);
                    if (ret == 0)
                        return false;
                    if (ret == SOCKET_ERROR) {
                        int err = WSAGetLastError();
                        if (err == WSAEWOULDBLOCK)
                            return true;
                        return false;
                    }
                #else
                #   warning unsupported OS
                #endif
                #ifdef OS_POSIX
                    int err = 0;
                    socklen_t errlen = sizeof(err);
                    if (getsockopt(s, SOL_SOCKET, SO_ERROR, &err, &errlen) < 0)
                        return false;
                    if (err != 0)
                        return false;
                #elif defined OS_WINDOWS
                    int err = 0;
                    int errlen = sizeof(err);
                    if (getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&err, &errlen) == SOCKET_ERROR)
                        return false;
                    if (err != 0)
                        return false;
                #endif
            }
            return true;
        } break;
        case SOCK_DGRAM:
            return true;
        default:
            log_msg(LOG_ERROR, "Unknown socket type: %d", so_type);
            return false;
    }
}
void network_stop(sock_t s) {
    if (s != SOCK_T_INVALID) {
        #if defined OS_POSIX
            shutdown(s, SHUT_RDWR);
            close(s);
        #elif defined OS_WINDOWS
            shutdown(s, SD_BOTH);
            closesocket(s);
        #else
        #   warning not implemented
        #endif
    }
}
int network_udp_wait_readable(sock_t s, int timeout_ms) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(s, &rfds);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    #ifdef OS_POSIX
        int nfds = (int)s + 1;
        int r = select(nfds, &rfds, NULL, NULL, &tv);
        return r;
    #elif defined OS_WINDOWS
        int r = select(0, &rfds, NULL, NULL, &tv);
        return r;
    #else
    #   warning OS Unsupported
        return -1;
    #endif
}

sock_t network_tcp_start(int *bound_port, int start_port, int backlog) {
    #if defined(OS_WINDOWS) && ! defined(OS_POSIX)
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return SOCK_T_INVALID;
    #endif

    sock_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return SOCK_T_INVALID;

    if (bind_any_available_port(server_fd, start_port, 32, bound_port) < 0) {
        close_sock(server_fd);
        #if defined(OS_WINDOWS) && ! defined(OS_POSIX)
            WSACleanup();
        #endif
        return SOCK_T_INVALID;
    }

    if (listen(server_fd, backlog) < 0) {
        close_sock(server_fd);
        #if defined(OS_WINDOWS) && ! defined(OS_POSIX)
            WSACleanup();
        #endif
        return SOCK_T_INVALID;
    }

    return server_fd;
}