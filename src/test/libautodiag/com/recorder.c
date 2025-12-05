#include "libTest.h"

bool testRecorder(VehicleIFace* iface) {

    viface_recorder_set_state(iface, false);
    viface_recorder_reset(iface);
    list_object_Record* recorder = viface_recorder_get(iface);
    assert(recorder->size == 0);
    viface_recorder_set_state(iface, true);
    viface_lock(iface);
    viface_send(iface, buffer_from_ints(0x01,0x01));
    viface_clear_data(iface);
    viface_recv(iface);
    assert(recorder->size == 1);

    return true;
}