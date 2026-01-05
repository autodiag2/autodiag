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
#include "libautodiag/sim/elm327/elm327.h"
#include <stdarg.h>
#include <assert.h>

static void initLibTest() {
    srand(time(NULL));
}

static void runTestWithMessage(char *msg, bool (*func)(VehicleIFace*), VehicleIFace* iface) {
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

static VehicleIFace* port_open(char *device_location) {
    printf("open port %s\n", device_location);
    final Serial * serial = serial_new();
    serial->location = strdup(device_location);
    return viface_open_from_device(AD_DEVICE(serial));
}
static SimELM327 * test_sim_elm327_new() {
    SimELM327* elm327 = sim_elm327_new();
    elm327->device_type = strdup("network");
    return elm327;
}
static char* start_elm327_simulation_with_ecus(SimECU *first) {
    SimELM327* elm327 = test_sim_elm327_new();
    if ( first != null ) {
        LIST_SIM_ECU(elm327->ecus)->list[0] = first;
    }
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    return strdup(elm327->device_location);
}

static char* start_elm327_simulation() {
    return start_elm327_simulation_with_ecus(null);    
}

static VehicleIFace* fake_can_iface() {
    VehicleIFace* tmp = viface_new();
    ELM327Device* device = elm327_new_from_serial(serial_new());
    tmp->device = (Device*)device;
    device->printing_of_spaces = false;
    device->protocol = ELM327_PROTO_ISO_15765_4_CAN_1;
    device->eol = strdup("\r");
    tmp->vehicle = vehicle_new();
    return tmp;
}
static VehicleIFace* fake_standard_obd_iface() {
    VehicleIFace* tmp = viface_new();
    ELM327Device* device = elm327_new_from_serial(serial_new());
    tmp->device = (Device*)device;    
    device->printing_of_spaces = false;
    device->protocol = ELM327_PROTO_SAE_J1850_1;
    device->eol = strdup("\r");
    tmp->vehicle = vehicle_new();
    return tmp;
}
#include <time.h>
#include <stdint.h>

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

#endif
