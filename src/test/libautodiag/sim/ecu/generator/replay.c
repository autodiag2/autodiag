#include "libTest.h"

bool testReplay() {
    {
        SimELM327* elm327 = sim_elm327_new();
        elm327->ecus[0].list[0]->generator = sim_ecu_generator_new_citroen_c5_x7();
        sim_elm327_loop_as_daemon(elm327);
        sim_elm327_loop_daemon_wait_ready(elm327);
        final VehicleIFace* iface = port_open(strdup(elm327->device_location));
        viface_recorder_reset(iface);
        viface_recorder_set_state(iface, true);
        viface_send_str(iface, "0101");
        viface_clear_data(iface);
        viface_recv(iface);
        viface_recorder_set_state(iface, false);
        char fpath[] = "/tmp/com.json";
        record_to_json_file(fpath);
        SimECUGenerator * g = sim_ecu_generator_new_replay(elm327->ecus[0].list[0]->address);
        g->context = strdup(fpath);
        elm327->ecus[0].list[0]->generator = g;
        viface_send_str(iface, "0101");
        viface_clear_data(iface);
        viface_recv(iface);
        assert(iface->vehicle->data_buffer->size == 1);
        assert(0 < iface->vehicle->data_buffer->list[0]->size);
        // Ensure that the index cycle when max index is reached
        viface_send_str(iface, "0101");
        viface_clear_data(iface);
        viface_recv(iface);
        assert(iface->vehicle->data_buffer->size == 1);
        assert(0 < iface->vehicle->data_buffer->list[0]->size);
    }
    {
        SimELM327* elm327 = sim_elm327_new();
        elm327->ecus[0].list[0]->generator = sim_ecu_generator_new_random();
        sim_elm327_loop_as_daemon(elm327);
        sim_elm327_loop_daemon_wait_ready(elm327);
        final VehicleIFace* iface = port_open(strdup(elm327->device_location));
        {
            viface_recorder_reset(iface);
            viface_recorder_set_state(iface, true);
            viface_send_str(iface, "0101");
            viface_clear_data(iface);
            viface_recv(iface);
            viface_send_str(iface, "0102");
            viface_clear_data(iface);
            viface_recv(iface);
            viface_send_str(iface, "0101");
            viface_clear_data(iface);
            viface_recv(iface);
            viface_recorder_set_state(iface, false);
            char fpath[] = "/tmp/com.json";
            record_to_json_file(fpath);
            SimECUGenerator * g = sim_ecu_generator_new_replay(elm327->ecus[0].list[0]->address);
            g->context = strdup(fpath);
            elm327->ecus[0].list[0]->generator = g;
            viface_send_str(iface, "0101");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 1);
            assert(0 < iface->vehicle->data_buffer->list[0]->size);
            viface_send_str(iface, "0102");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 1);
            assert(0 < iface->vehicle->data_buffer->list[0]->size);
            viface_send_str(iface, "0101");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 1);
            assert(0 < iface->vehicle->data_buffer->list[0]->size);
            viface_send_str(iface, "0101");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 1);
            assert(0 < iface->vehicle->data_buffer->list[0]->size);
            viface_send_str(iface, "0103");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 0);
        }
        {
            SimECUGenerator * g = sim_ecu_generator_new_replay();
            char json[] = "{"
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
            "}";
            g->context = json;
            elm327->ecus[0].list[0]->generator = g;
            viface_send_str(iface, "0101");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 1);
            assert(0 < iface->vehicle->data_buffer->list[0]->size);
            viface_send_str(iface, "0102");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 1);
            assert(0 < iface->vehicle->data_buffer->list[0]->size);
            viface_send_str(iface, "0101");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 1);
            assert(0 < iface->vehicle->data_buffer->list[0]->size);
            viface_send_str(iface, "0101");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 1);
            assert(0 < iface->vehicle->data_buffer->list[0]->size);
            viface_send_str(iface, "0103");
            viface_clear_data(iface);
            viface_recv(iface);
            assert(iface->vehicle->data_buffer->size == 0);
        }
    }
    return true;
}