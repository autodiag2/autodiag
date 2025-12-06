#include "libTest.h"
#include "libautodiag/sim/sim.h"

bool testSim() {
    SimELM327* elm327 = sim_elm327_new();
    assert(sim_load_from_json(SIM(elm327), 
        "["
            "{"
                "\"ecu\": \"07e8\","
                "\"flow\": ["
                    "{"
                        "\"request\": \"0101\","
                        "\"responses\": [\"41010f47676687\"]"
                    "},"
                    "{"
                        "\"request\": \"0102\","
                        "\"responses\": [\"410259aa883c59\"]"
                    "},"
                    "{"
                        "\"request\": \"0101\","
                        "\"responses\": [\"4101ea56137bd2\"]"
                    "}"
                "]"
            "},"
            "{"
                "\"ecu\": \"07e9\","
                "\"flow\": ["
                    "{"
                        "\"request\": \"0101\","
                        "\"responses\": [\"41010f47676687\"]"
                    "},"
                    "{"
                        "\"request\": \"0103\","
                        "\"responses\": [\"4103ea56137bd2\"]"
                    "}"
                "]"
            "}"
        "]"
    ) == GENERIC_FUNCTION_SUCCESS );
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = port_open(strdup(elm327->device_location));
    assert(LIST_SIM_ECU(elm327->ecus)->size == 2);
    viface_lock(iface);
    viface_send_str(iface, "0101");
    viface_clear_data(iface);
    viface_recv(iface);
    assert(iface->vehicle->data_buffer->size == 2);
    viface_send_str(iface, "0102");
    viface_clear_data(iface);
    viface_recv(iface);
    assert(iface->vehicle->data_buffer->size == 1);
    viface_send_str(iface, "0103");
    viface_clear_data(iface);
    viface_recv(iface);
    assert(iface->vehicle->data_buffer->size == 1);
    viface_send_str(iface, "0104");
    viface_clear_data(iface);
    viface_recv(iface);
    assert(iface->vehicle->data_buffer->size == 0);
    viface_unlock(iface);
    return true;
}