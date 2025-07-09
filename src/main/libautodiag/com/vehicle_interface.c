#include "libautodiag/com/vehicle_interface.h"
#include "libautodiag/com/serial/elm/elm.h"

void viface_close(final VehicleIFace* iface) {
    iface->device->close(iface->device);    
}

int viface_send(final VehicleIFace* iface, const char *request) {
    return iface->device->send(iface->device, request);
}
void viface_free(final VehicleIFace* iface) {
    assert(iface != null);
    if ( iface->vehicle != null ) {
        vehicle_free(iface->vehicle);
    }
    if ( iface->device != null ) {
        log_msg(LOG_ERROR, "should free the device");
    }
    free(iface);
}

VehicleIFace* viface_new() {
    final VehicleIFace* iface = (VehicleIFace*)malloc(sizeof(VehicleIFace));
    iface->device = null;
    iface->vehicle = vehicle_new();
    return iface;
}

void viface_lock(final VehicleIFace* iface) {
    iface->device->lock(iface->device);
}

void viface_unlock(final VehicleIFace* iface) {
    iface->device->unlock(iface->device);
}

VehicleIFace* viface_open_from_device(final Device* device) {
    assert(device->type != null);
    final VehicleIFace* iface = viface_new();
    if ( strcmp(device->type, "serial") == 0 ) {
        Serial * serial = (Serial*)device;
        // TODO should test if ati contains ELM, if so, launch elm open from serial
        ELMDevice * elm = elm_open_from_serial(serial);
        if ( elm == null ) {
            log_msg(LOG_ERROR, "Cannot open ELM interface from serial port %s: device config has failed", serial->location);
            return null;
        }
        iface->device = CAST_DEVICE(elm);
    } else {
        log_msg(LOG_ERROR, "Unknown device type: %s aborting", device->type);
        viface_free(iface);
        return null;
    }

    obd_discover_vehicle(iface);
    return iface;
}