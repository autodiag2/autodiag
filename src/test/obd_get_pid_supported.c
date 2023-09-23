#include "lib.h"
#define _GNU_SOURCE
#include "com/serial/obd/obd.h"
#include <stdio.h>
#include "com/serial/serial.h"
#include "com/serial/obd/saej1979/saej1979.h"
#include <string.h>
#include <assert.h>

int main(int argc, char **argv) {
    OBDIFACE port = port_open(start_simulation());

    printf("PID full support table:\n");
    char * res = strdup(""), *tmp;
    for(int i = 1; i <= 0x20; i++) {
        if ( saej1979_is_pid_supported(port,i) ) {
            asprintf(&tmp,"%s%02x ",res,i);
            free(res);
            res = tmp;
        }
    }
    printf("%s\n", res);
    
    port_close_destruct_simulation(port);
    return 0;
}
