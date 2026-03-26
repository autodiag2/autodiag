#include "libTest.h"
#include "libautodiag/sim/elm327/elm327.h"

static uint16_t some_signal = 10;

static Buffer * response(SimECUGenerator * g, Buffer * binRequest) {
    if ( binRequest->buffer[0] == 0x10 ) {
        return ad_buffer_be_from_uint16(some_signal);
    }
    return ad_buffer_new_random(10);
}

bool testSignals() {
    SimELM327* elm327 = tf_sim_elm327_new();
    SimECUGenerator * g = sim_ecu_generator_new_citroen_c5_x7();
    g->response = response;
    elm327->ecus->list[0]->generator = g;
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = tf_serial_open(strdup(elm327->device_location));

    ad_object_vehicle_signal * signal = ad_object_vehicle_signal_new();
    signal->rv_formula = strdup("$0 * 256 + $1");
    signal->rv_min = 0;
    signal->rv_max = 100;
    signal->input = ad_buffer_from_ascii_hex("10");
    signal->unit = strdup("some");
    for(int i = 10; i < 100; i += 20) {
        some_signal = i;
        double result = 0;
        assert(viface_use_signal(iface, signal, &result));
        assert(result == some_signal);
    }
    return true;
}