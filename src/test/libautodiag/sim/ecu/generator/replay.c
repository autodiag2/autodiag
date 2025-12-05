#include "libTest.h"

bool testReplay() {
    SimELM327* elm327 = sim_elm327_new();
    elm327->ecus[0].list[0]->generator = sim_ecu_generator_new_citroen_c5_x7();
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = port_open(strdup(elm327->device_location));
    viface_recorder_set_state(iface, true);
    viface_send_str(iface, "0101");
    viface_clear_data(iface);
    viface_recv(iface);
    viface_recorder_set_state(iface, false);
    char fpath[] = "/tmp/com.json";
    record_to_json_file(fpath);
    SimECUGenerator * g = sim_ecu_generator_new_replay(
        buffer_from_ints(0x07, elm327->ecus[0].list[0]->address)
    );
    g->context = strdup(fpath);
    elm327->ecus[0].list[0]->generator = g;
    viface_send_str(iface, "0101");
    viface_clear_data(iface);
    viface_recv(iface);
    assert(iface->vehicle->data_buffer->size == 1);
    assert(0 < iface->vehicle->data_buffer->list[0]->size);
    return true;
}