#include "libTest.h"

bool testCycle() {
    SimELM327* elm327 = sim_elm327_new();
    SimECUGenerator * generator = sim_ecu_generator_new_cycle();
    LIST_SIM_ECU(elm327->ecus)[0].list[0]->generator = generator;
    generator->context_load_from_string(generator, "10");
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);

    final VehicleIFace* iface = port_open(strdup(elm327->device_location));
    viface_send_str(iface, "0101");
    viface_clear_data(iface);
    viface_recv(iface);
    assert(iface->vehicle->data_buffer->size == 1);
}