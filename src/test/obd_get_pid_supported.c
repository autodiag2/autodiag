#include "libTest.h"

int main(int argc, char **argv) {
    OBDIFace* port = port_open(start_simulation());

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
    
    port_close_destruct_simulation(port);
    return 0;
}
