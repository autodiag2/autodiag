#include "libTest.h"

int main(int argc, char **argv) {
    SimELM327* elm327 = elm327_sim_new();
    SimECU_list_append(elm327->ecus,sim_ecu_emulation_new(0xE9));        
    elm327_sim_loop_as_daemon(elm327);
    usleep(200e3);
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
    elm327_sim_destroy(elm327);
    return 0;
}
