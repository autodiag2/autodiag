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