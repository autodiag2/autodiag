#include "libautodiag/sim/ecu/ecu.h"

int SimECU_cmp(SimECU* e1, SimECU* e2) {
    return e1 - e2;
}
AD_LIST_SRC(SimECU)

void list_SimECU_empty(list_SimECU * list) {
    while(0 < list->size) {
        log_msg(LOG_DEBUG, "should free ecu here");
        list_SimECU_remove_at(list, 0);
    }
}
/**
 * Respond to use request (at commands, OBD, UDS)
 */
Buffer * sim_ecu_response(SimECU * ecu, Buffer * binRequest) {
    final Buffer* binResponse;
    if ( ecu->generator->response_for_python != null ) {
        binResponse = buffer_new();
        ecu->generator->response_for_python(ecu->generator, binRequest, binResponse);
    } else {
        binResponse = ecu->generator->response(ecu->generator, binRequest);
    }
    return binResponse;
}

SimECU* sim_ecu_new(byte address) {
    final SimECU* emu = (SimECU*)malloc(sizeof(SimECU));
    emu->address = address;
    emu->generator = sim_ecu_generator_new_random();
    return emu;
}
