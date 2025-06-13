#ifndef __TEST_LIB_H
#define __TEST_LIB_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int asprintf(char **strp, const char *fmt, ...);

#include "libautodiag/com/serial/serial.h"
#include "libautodiag/com/obd/obd.h"
#include "libautodiag/string.h"
#include <sys/types.h>
#include <signal.h>
#include "libautodiag/sim/elm327/sim.h"
#include <stdarg.h>
#include <assert.h>

static void initLibTest() {
    srand(time(NULL));
}

static void runTestWithMessage(char *msg, bool (*func)(OBDIFace*), OBDIFace* iface) {
    printf("Running '%s'\n", msg);
    printf("   -> %s\n",func(iface) ? "\033[32mSUCCESS\033[0m" : "\033[33mFAILURE\033[0m");
}

#define runTest(func,iface) runTestWithMessage(#func,func,iface)

static bool shouldRun(int argc, char **argv, char *element) {
    if ( argc == 1 ) {
        return true;
    }
    for(int i = 1; i < argc; i ++) {
        if ( strcmp(argv[i],element) == 0 ) {
            return true;
        }
    }
    return false;
}

#define runTestMaybe(func,iface) \
    if ( shouldRun(argc,argv,#func) ) { \
        runTest(func,iface); \
    }

static void testOutput(char *fmt, ...) {
    char *result;
    asprintf(&result, "   %s\n", fmt);
    va_list ap;

    va_start(ap, fmt);
    vprintf(result, ap);

    va_end(ap);
    free(result);
}

static FILE* tmpFile(char **name) {
    asprintf(name,"/tmp/file_%d%d%d",rand(),rand(),rand());
    return fopen(*name,"w+");
}

static OBDIFace* port_open(char *port_location) {
    printf("open port %s\n", port_location);
    final SERIAL serial = serial_new();
    serial->name = strdup(port_location);
    return elm_open_from_serial(serial);
}

static OBDIFace* port_parse_open(int argc, char **argv) {
    char * port_location = "/dev/pts/2";
    if ( 1 < argc ) {
        port_location = argv[1];
    }
    return port_open(port_location);
}

static char* start_elm327_simulation() {
    ELM327emulation* elm327 = elm327_sim_new();
    elm327_sim_loop_as_daemon(elm327);
    usleep(200e3);
    return strdup(elm327->device_location);
}

static OBDIFace* fake_can_iface() {
    OBDIFace* tmp = obd_new();
    ELM327Device* device = elm327_new_from_serial(serial_new());
    tmp->device = (Device*)device;
    device->printing_of_spaces = false;
    device->protocol = ELM327_PROTO_ISO_15765_4_CAN_1;
    device->eol = strdup("\r");
    tmp->vehicle = vehicle_new();
    return tmp;
}
static OBDIFace* fake_standard_obd_iface() {
    OBDIFace* tmp = obd_new();
    ELM327Device* device = elm327_new_from_serial(serial_new());
    tmp->device = (Device*)device;    
    device->printing_of_spaces = false;
    device->protocol = ELM327_PROTO_SAE_J1850_1;
    device->eol = strdup("\r");
    tmp->vehicle = vehicle_new();
    return tmp;
}


#endif
