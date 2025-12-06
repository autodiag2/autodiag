#include "libautodiag/sim/elm327/sim_ecu.h"
#include "libautodiag/sim/elm327/elm327.h"

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

char * sim_ecu_generate_request_header_bin(struct _SimELM327* elm327,byte source_address, byte can28bits_prio, bool print_spaces) {
    char *protocolSpecificHeader = null;
    char * space = print_spaces ? " " : "";
    if ( elm327_protocol_is_can(elm327->protocolRunning) ) {
        if ( elm327_protocol_is_can_29_bits_id(elm327->protocolRunning) ) {
            asprintf(&protocolSpecificHeader,"%02X%sDA%sF1%s%02hhX", can28bits_prio, space, space, space,source_address);
        } else if ( elm327_protocol_is_can_11_bits_id(elm327->protocolRunning) ) {
            asprintf(&protocolSpecificHeader,"7%02hhX",source_address);
        } else {
            log_msg(LOG_WARNING, "Missing case here");
        }
        if ( elm327->can.extended_addressing ) {
            char *tmp;
            asprintf(&tmp,"%s%s%02hhX", protocolSpecificHeader, space, elm327->can.extended_addressing_target_address);
            free(protocolSpecificHeader);
            protocolSpecificHeader = tmp;
        }
    } else {
        asprintf(&protocolSpecificHeader,"41%s6B%s%02hhX", space,space,source_address);
    }
    return protocolSpecificHeader;     
}

Buffer * sim_ecu_response_generic(SimELM327 * elm327, SimECU * ecu, Buffer * binRequest) {
    final Buffer* binResponse;
    if ( ecu->generator->response_for_python != null ) {
        binResponse = buffer_new();
        ecu->generator->response_for_python(ecu->generator, binRequest, binResponse);
    } else {
        binResponse = ecu->generator->response(ecu->generator, binRequest);
    }
    return binResponse;
}

SimECU* sim_ecu_emulation_new(byte address) {
    final SimECU* emu = (SimECU*)malloc(sizeof(SimECU));
    emu->sim_ecu_response = (Buffer *(*)(struct _SimELM327 *, struct SimECU *, Buffer *))sim_ecu_response_generic;
    emu->address = address;
    emu->generator = sim_ecu_generator_new_random();
    return emu;
}
