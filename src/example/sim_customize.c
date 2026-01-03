#include <libautodiag/sim/elm327/elm327.h>
#include <stdio.h>
#include <string.h>

static Buffer * response(struct SimECUGenerator * this, final Buffer *binRequest) {
    final Buffer * binResponse = buffer_new();
    if ( 0 < binRequest->size ) {
        switch(binRequest->buffer[0]) {
            case OBD_SERVICE_SHOW_CURRENT_DATA: {
                // positive response
                buffer_append_byte(binResponse, OBD_SERVICE_SHOW_CURRENT_DATA + OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE);
                // example: return 0 for all PIDs
                for(int i = 2; i < 8; i++) {
                    buffer_append_byte(binResponse, 0x00);
                }
            } break;
            default:
                buffer_append_byte(binResponse, 0x7F); // negative response
                buffer_append_byte(binResponse, binRequest->buffer[0]); // echo request
                buffer_append_byte(binResponse, 0x12); // subfunction not supported
                break;
        }
    }
    return binResponse;
}

int main(int argc, char ** argv) {
    SimELM327* sim = sim_elm327_new();
    assert(0 < sim->ecus->size);
    SimECU * ecu = sim->ecus->list[0];
    SimECUGenerator * generator = ecu->generator;
    generator->response = response;
    sim_elm327_loop_as_daemon(sim);
    sim_elm327_loop_daemon_wait_ready(sim);
    printf("Simulation started at: %s\n", sim->device_location);
    while(1) {
        sleep(1);
    }
    return 0;
}