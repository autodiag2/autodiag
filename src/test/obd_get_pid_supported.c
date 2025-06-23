#include "libTest.h"

int main(int argc, char **argv) {
    SimELM327* elm327 = sim_elm327_new();
    list_SimECU_append(elm327->ecus,sim_ecu_emulation_new(0xE9));        
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final OBDIFace* port = port_open(strdup(elm327->device_location));

    printf("PID full support table:\n");
    char * res = strdup(""), *tmp;
    for(int i = 1; i <= 0x20; i++) {
        if ( saej1979_data_is_pid_supported(port,false,i) ) {
            asprintf(&tmp,"%s%02x ",res,i);
            free(res);
            res = tmp;
        }
    }
    printf("%s\n", res);
    sim_elm327_destroy(elm327);
    return 0;
}
