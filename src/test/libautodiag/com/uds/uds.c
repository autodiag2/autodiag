#include "libTest.h"
#include "libautodiag/sim/elm327/sim_generators.h"
#include "libautodiag/com/uds/uds.h"

bool testUDS() {
    SimECU * ecu = sim_ecu_emulation_new(0xE8);
    ecu->generator = sim_ecu_generator_new_citroen_c5_x7();
    final VehicleIFace* iface = port_open(start_elm327_simulation_with_ecus(ecu));
    {
        assert(uds_request_session_cond(iface, UDS_SESSION_DEFAULT));
        assert(uds_request_session_cond(iface, UDS_SESSION_EXTENDED_DIAGNOSTIC));
        assert(uds_request_session_cond(iface, UDS_SESSION_PROGRAMMING));
        assert(uds_request_session_cond(iface, UDS_SESSION_SYSTEM_SAFETY_DIAGNOSTIC));
    }
    return true;
}