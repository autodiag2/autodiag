#include "libTest.h"
#include "libautodiag/com/obdb/obdb.h"

static Buffer *(*parent_response)(SimECUGenerator *, Buffer *) = null;
static int dtc_count = 2;
static Buffer *response_wrapper(SimECUGenerator *generator, Buffer *binRequest) {
    assert(parent_response != null);
    if ( binRequest->buffer[0] == 0x01 ) {
        if ( binRequest->buffer[1] == 0x01 ) {
            return ad_buffer_from_ascii_hex(gprintf("4101%02hhX000000", dtc_count));
        }
    }
    return parent_response(generator, binRequest);
}
void test_real_life() {
    SimECU * ecu = sim_ecu_new(0xE8);
    SimECUGenerator * g = sim_ecu_generator_new_citroen_c5_x7();
    parent_response = g->response;
    g->response = response_wrapper;
    ecu->generator = g;
    final VehicleIFace* iface = tf_serial_open(tf_sim_elm327_start_with_ecu(ecu));
    assert(ad_obdb_fetch_signals("SAEJ1979"));
    ad_object_vehicle_signal * signal = ad_signal_get("SAEJ1979.DTC_CNT");
    assert(signal != null);
    {
        double result = NAN;
        dtc_count = 2;
        assert(viface_use_signal(iface, signal, &result, null));
        assert(result == 2.0);
    }
}
bool testOBDB() {
    assert(ad_obdb_fetch_signals("SAEJ1979"));
    assert(ad_obdb_fetch_signals("ACURA-TLX"));
    ad_object_vehicle_signal * signal = ad_signal_get("OBDb.TLX_GENERATOR");
    assert(signal != null);
    tf_test_output("OBDb.tlx_generator: formula = %s\n", signal->rv_formula);
    ad_object_vehicle_signal_dump(signal);
    tf_run_case(test_real_life);
    return true;
}