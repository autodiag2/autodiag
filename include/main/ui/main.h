#ifndef __MAIN_H
#define __MAIN_H

#define MODULE_MAIN "Main", 

#define _GNU_SOURCE
#include <stdio.h>
#include "ui.h"
#include "libautodiag/lib.h"
#include "libprog/config.h"
#include "libautodiag/log.h"
#include "libautodiag/com/serial/serial.h"
#include "libautodiag/com/obd/obd.h"
#include "libautodiag/lib.h"
#include "libautodiag/math.h"

#include "trouble_code_reader.h"
#include "options.h"
#include "CommandLine.h"
#include "vehicle_explorer.h"
#include "documentation.h"

typedef struct {
    GtkWidget *window;
    struct {
        struct {
            GtkSpinner *waitIcon;
            GtkLabel *text;
            struct {
                GtkLabel *protocole, *interface_name;
                GtkBox *container;
            } more;
        } state;
    } adaptater;
    struct {
        GtkLabel *manufacturer;
        GtkLabel *country;
        GtkLabel *year;
        GtkLabel *vin;
    } vehicle;
} MainGui;

extern MainGui *mainGui;

void refresh_usb_adaptater_state();
void*refresh_usb_adaptater_state_spinner(void *arg);
void* refresh_usb_adaptater_state_internal(void *arg);
void main_usb_adaptater_state_spinner_wait_for(void* (*function)(void*arg));
void module_init_main();
void debug_show_config();

#endif
