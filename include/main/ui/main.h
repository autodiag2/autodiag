#ifndef __MAIN_H
#define __MAIN_H

#define MODULE_MAIN "Main", 

#define _GNU_SOURCE
#include <stdio.h>
#include <gtk/gtk.h>
#include "globals.h"
#include "config.h"
#include "log.h"
#include "ui/trouble_code_reader.h"
#include "ui/options.h"
#include "com/serial/serial.h"
#include "com/serial/obd/obd.h"
#include "globals.h"
#include "ui/CommandLine.h"
#include "ui/vehicle_explorer.h"
#include "ui/documentation.h"

typedef struct {
    GtkWidget *window;
    struct {
        struct {
            GtkSpinner *waitIcon;
            GtkLabel *text;
            struct {
                GtkLabel *protocole, *interface;
                GtkBox *container;
            } more;
        } state;
    } adaptater;
} MainGui;

extern MainGui *maingui;
extern GtkWidget *windowAbout, *windowFreezeFrame, *windowSensorData;

void refresh_usb_adaptater_state();
void*refresh_usb_adaptater_state_spinner(void *arg);
void* refresh_usb_adaptater_state_internal(void *arg);
void main_usb_adaptater_state_spinner_wait_for(void* (*function)(void*arg));

#endif
