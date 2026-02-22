#include "libautodiag/time.h"

uint64_t time_ms() {
#ifdef OS_WINDOWS
    return GetTickCount64();
#elif defined OS_POSIX
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
#else
#   warning OS Unsupported
#endif
}