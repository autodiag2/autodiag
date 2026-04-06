#include "libTest.h"
#include "libautodiag/sim/elm327/elm327.h"
#include "libautodiag/com/obd/saej1979/data.h"

static uint16_t some_signal = 10;
static double engine_load = 55.0;

static Buffer * response(SimECUGenerator * g, Buffer * binRequest) {
    if ( binRequest->buffer[0] == 0x10 ) {
        return ad_buffer_be_from_uint16(some_signal);
    }
    Buffer * binResponse = ad_buffer_new();
    if ( binRequest->buffer[0] == 0x01) {
        if ( binRequest->buffer[1] == 0x04 ) {
            AD_BUFFER_APPEND_BYTES(binResponse, 0x41, 0x04, (byte)(engine_load * 2.55));
        }
        return binResponse;
    }
    if ( binRequest->buffer[0] == 0x02 ) {
        switch(binRequest->buffer[1]) {
            case 0x04: {
                final int frameNumber = binRequest->buffer[2];
                if ( frameNumber == 0x01 ) {
                    AD_BUFFER_APPEND_BYTES(binResponse, 0x42, 0x04, 0x01, (byte)(10 * 2.55));
                }
                if ( frameNumber == 0x02 ) {
                    AD_BUFFER_APPEND_BYTES(binResponse, 0x42, 0x04, 0x02, (byte)(20 * 2.55));
                }
            } break;
        }
        return binResponse;
    }
    return ad_buffer_new_random(10);
}
static int response_bytes_offset(Buffer * buffer, char * input_formula) {
    if ( buffer->size >= 3 && buffer->buffer[0] == 0x41 ) {
        return 2;
    }
    if ( buffer->size >= 4 && buffer->buffer[0] == 0x42 ) {
        return 3;
    }
    return 0;
}
void testRawSignal() {
    SimELM327* elm327 = tf_sim_elm327_new();
    SimECUGenerator * g = sim_ecu_generator_new_citroen_c5_x7();
    g->response = response;
    elm327->ecus->list[0]->generator = g;
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = tf_serial_open(strdup(elm327->device_location));

    {
        ad_object_vehicle_signal * signal = ad_object_vehicle_signal_new();
        signal->rv_formula = strdup("$0 * 256 + $1");
        signal->rv_min = 0;
        signal->rv_max = 100;
        signal->input_formula = strdup("10");
        signal->unit = strdup("some");
        signal->rv_offset_bytes = response_bytes_offset;
        for(int i = 10; i < 100; i += 20) {
            some_signal = i;
            double result = 0;
            assert(viface_use_signal(iface, signal, &result, null));
            assert(round(result) == round(some_signal));
        }
    }
    {
        ad_saej1979_data_register_signals();
        ad_object_vehicle_signal * signal = ad_signal_get("SAEJ1979.engine_load");
        assert(signal);
        double result = 0;
        assert(viface_use_signal(iface, signal, &result, "01", null));
        assert(round(result) == round(engine_load));
        assert(viface_use_signal(iface, signal, &result, "02", "01", null));
        assert(round(result) == round(10));
        assert(viface_use_signal(iface, signal, &result, "02", "02", null));
        assert(round(result) == round(20));
    }
}
bool testSignals() {
    tf_run_case(testRawSignal);
    return true;
}