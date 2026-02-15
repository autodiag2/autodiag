#include "libautodiag/com/device.h"

bool device_location_is_network(final Device *device) {
    if (device == null || device->location == null) {
        return false;
    }

    const char *s = device->location;
    int dots = 0;

    while (*s) {
        if (isdigit((unsigned char)*s)) {
            int v = 0;
            while (isdigit((unsigned char)*s)) {
                v = v * 10 + (*s - '0');
                if (v > 255) return false;
                s++;
            }
            if (*s == '.') {
                dots++;
                s++;
            } else {
                break;
            }
        } else {
            return false;
        }
    }

    if (dots != 3) return false;

    if (*s == ':') {
        s++;
        int p = 0;
        int digits = 0;
        while (isdigit((unsigned char)*s)) {
            p = p * 10 + (*s - '0');
            if (p > 65535) return false;
            s++;
            digits++;
        }
        if (digits == 0) return false;
    }

    return *s == '\0';
}