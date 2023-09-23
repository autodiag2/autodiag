#define _GNU_SOURCE
#include <stdio.h>

#include "com/serial/serial.h"
#include "com/serial/obd/obd.h"
#include "com/serial/obd/saej1979/saej1979.h"
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <assert.h>

static int currentSimulation = 0;

void runTest(char *msg, bool (*func)(OBDIFACE), OBDIFACE iface) {
    printf("Running '%s'\n", msg);
    printf("   -> %s\n",func(iface) ? "SUCCESS" : "FAILURE");
}

static OBDIFACE port_open(char *port_location) {
    printf("open port %s\n", port_location);
    final SERIAL serial = serial_new();
    serial->name = strdup(port_location);
    serial_open(serial);
    OBDIFACE iface = null;
    for(int i = 0; i < 5 && iface == null;i++) 
        iface = obd_open_from_serial(serial);
    return iface;
}

static OBDIFACE port_parse_open(int argc, char **argv) {
    char * port_location = "/dev/pts/2";
    if ( 1 < argc ) {
        port_location = argv[1];
    }
    return port_open(port_location);
}

static void port_close(OBDIFACE iface) {
    obd_close(iface);
    obd_free(iface);
}

static void port_close_destruct_simulation(OBDIFACE iface) {
    port_close(iface);
    kill(currentSimulation,SIGKILL);
    kill(currentSimulation,SIGSEGV);
}

static char* start_simulation() {
    struct dirent **namelist, **namelist2;
    final int tts_count = scandir("/dev/pts/", &namelist,NULL,&alphasort);

    char *simTTY = NULL;
    final int pid = fork();
    if ( pid == 0 ) {
        execl("/usr/bin/obdsim", "obdsim", (char  *) NULL);
    } else {
        currentSimulation = pid;
        int tts_count2 = -1;
        while((tts_count2 = scandir("/dev/pts/", &namelist2,NULL,&alphasort)) == tts_count) {
            sleep(2);
        }
        for(int i = 0; i < tts_count2; i++) {
            bool found = false;
            for(int j = 0; j < tts_count; j++) {
                if (strcmp(namelist2[i]->d_name,namelist[j]->d_name) == 0) {
                    found = true;
                    break;
                }
            }
            if ( ! found ) {
                asprintf(&simTTY,"/dev/pts/%s", namelist2[i]->d_name);
            }
        }
    }
    return simTTY;
}

static OBDIFACE port_open_on_new_simulation() {
    log_set_from_env();
    return port_open(start_simulation());
}

