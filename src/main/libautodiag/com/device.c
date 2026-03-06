#include "libautodiag/com/device.h"

const char * device_describe_state(final Device * device) {
    switch(device->state) {
        case AD_DEVICE_STATE_UNDEFINED:
            return "Undefined state";
        case AD_DEVICE_STATE_READY:
            return "Ready";
        case AD_DEVICE_STATE_NOT_READY:
            return "Not ready";
        default: break;
    }
    return null;
}
bool ad_device_location_is_network(char *location, char **ip_rv, int *port_rv) {
    const char *s = location;
    const char *ip_start = location;
    const char *port_start = null;
    int dots = 0;
    int port = -1;

    while (*s) {
        if (isdigit((unsigned char)*s)) {
            int v = 0;
            while (isdigit((unsigned char)*s)) {
                v = v * 10 + (*s - '0');
                if (255 < v) return false;
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
        port_start = s;
        int p = 0;
        int digits = 0;
        while (isdigit((unsigned char)*s)) {
            p = p * 10 + (*s - '0');
            if (65535 < p) return false;
            s++;
            digits++;
        }
        if (digits == 0) return false;
        port = p;
    }

    if (*s != '\0') return false;

    if (ip_rv != null) {
        size_t len = (port_start != null ? (size_t)(port_start - ip_start - 1) : strlen(ip_start));
        char *ip = malloc(len + 1);
        memcpy(ip, ip_start, len);
        ip[len] = '\0';
        *ip_rv = ip;
    }

    if (port_rv != null) {
        *port_rv = (port < 0 ? 0 : port);
    }

    return true;
}
bool device_is_network(final Device *device) {
    if (device == null || device->location == null) {
        return false;
    }
    return ad_device_location_is_network(device->location, null, null);
}
AD_DEVICE_TYPE ad_device_type_from_location(char * location) {
    char * ip;
    int port;
    if ( ad_device_location_is_network(location, &ip, &port) ) {
        free(ip);
        if ( 35000 <= port ) {
            return AD_DEVICE_TYPE_SERIAL;
        }
        if ( 13400 <= port ) {
            return AD_DEVICE_TYPE_DOIP;
        }
    }
    return AD_DEVICE_TYPE_SERIAL;
}
AD_DEVICE_TYPE ad_device_type_from_str(char * type_str) {
    if ( strcasecmp(type_str, "auto") == 0 ) {
        return AD_DEVICE_TYPE_AUTO;
    } else if ( strcasecmp(type_str, "serial") == 0 || strcasecmp(type_str, "elm") == 0 ) {
        return AD_DEVICE_TYPE_SERIAL;
    } else if ( strcasecmp(type_str, "doip") == 0 ) {
        return AD_DEVICE_TYPE_DOIP;
    } else {
        log_msg(LOG_WARNING, "Unknown device type selected '%s', fallback to auto", type_str);
        return AD_DEVICE_TYPE_AUTO;
    }
}
const char * ad_device_type_as_string(AD_DEVICE_TYPE type) {
    switch(type) {
        case AD_DEVICE_TYPE_SERIAL: return "Serial";
        case AD_DEVICE_TYPE_DOIP: return "DoIP";
        default: return "Unknown";
    }
}
void device_location_set(final Device * device, final char *location) {
    assert(location != null);
    if ( device->location != null ) {
        free(device->location);
    }
    device->location = strdup(location);
}