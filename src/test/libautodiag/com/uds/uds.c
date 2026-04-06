#include "libTest.h"
#include "libautodiag/sim/ecu/generator.h"
#include "libautodiag/com/uds/uds.h"

bool testUDS() {
    SimECU * ecu = sim_ecu_new(0xE8);
    ecu->generator = sim_ecu_generator_new_citroen_c5_x7();
    final VehicleIFace* iface = tf_serial_open(tf_sim_elm327_start_with_ecu(ecu));
    {
        assert(ad_uds_request_session_cond(iface, UDS_SESSION_DEFAULT));
        assert(ad_uds_request_session_cond(iface, UDS_SESSION_EXTENDED_DIAGNOSTIC));
        assert(ad_uds_request_session_cond(iface, UDS_SESSION_PROGRAMMING));
        assert(ad_uds_request_session_cond(iface, UDS_SESSION_SYSTEM_SAFETY_DIAGNOSTIC));
    }
    return true;
}