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
#include "libautodiag/sim/doip/doip.h"
#include <stdarg.h>
#include <assert.h>

static void tf_init() {
    srand(time(NULL));
}

static void tf_test_run_with_message(char *msg, bool (*func)(VehicleIFace*), VehicleIFace* iface) {
    printf("Running '%s'\n", msg);
    printf("   -> %s\n",func(iface) ? "\033[32mSUCCESS\033[0m" : "\033[33mFAILURE\033[0m");
}

static bool tf_test_should_run(int argc, char **argv, char *element) {
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

#define runIfSelected(func,iface) \
    if ( tf_test_should_run(argc,argv,#func) ) { \
        tf_test_run_with_message(#func,func,iface); \
    }

static void tf_test_output(char *fmt, ...) {
    char *result;
    asprintf(&result, "   %s\n", fmt);
    va_list ap;

    va_start(ap, fmt);
    vprintf(result, ap);

    va_end(ap);
    free(result);
}

static FILE* tf_get_tmp_file(char **name) {
    asprintf(name,"/tmp/file_%d%d%d",rand(),rand(),rand());
    return fopen(*name,"w+");
}

static VehicleIFace* tf_serial_open(char *device_location) {
    printf("open port %s\n", device_location);
    final Serial * serial = serial_new();
    serial->location = strdup(device_location);
    return viface_open_from_device(AD_DEVICE(serial));
}
static VehicleIFace* tf_doip_open(char *device_location) {
    printf("open doip %s\n", device_location);
    final object_DoIPDevice * device = object_DoIPDevice_new();
    device->location = strdup(device_location);
    return viface_open_from_device(AD_DEVICE(device));
}
static char* tf_sim_doip_start_with_ecu(SimECU *first) {
    SimDoIp* sim = sim_doip_new();
    if ( first != null ) {
        LIST_SIM_ECU(sim->ecus)->list[0] = first;
    }
    sim_doip_loop_as_daemon(sim);
    sim_doip_loop_daemon_wait_ready(sim);
    return strdup(sim->device_location);
}
static char * tf_sim_doip_start() {
    SimECU * ecu = sim_ecu_new(0xE8);
    ecu->generator = sim_ecu_generator_new_citroen_c5_x7();
    return tf_sim_doip_start_with_ecu(ecu);
}
static SimELM327 * tf_sim_elm327_new() {
    SimELM327* elm327 = sim_elm327_new();
    elm327->device_type = strdup("network");
    return elm327;
}
static char* tf_sim_elm327_start_with_ecu(SimECU *first) {
    SimELM327* elm327 = tf_sim_elm327_new();
    if ( first != null ) {
        LIST_SIM_ECU(elm327->ecus)->list[0] = first;
    }
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    return strdup(elm327->device_location);
}
static char* tf_sim_elm327_start() {
    return tf_sim_elm327_start_with_ecu(null);    
}

static VehicleIFace* tf_fake_can_iface() {
    VehicleIFace* tmp = viface_new();
    ELM327Device* device = elm327_new_from_serial(serial_new());
    tmp->device = (Device*)device;
    device->printing_of_spaces = false;
    device->protocol = ELM327_PROTO_ISO_15765_4_CAN_1;
    device->eol = strdup("\r");
    tmp->vehicle = vehicle_new();
    return tmp;
}

#include <time.h>
#include <stdint.h>

static inline uint64_t tf_now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

#endif
