#ifndef __ELM327_CLI_H
#define __ELM327_CLI_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib/lib.h"
#include "sim.h"
#include "ui/gtk.h"
#include "ui/counter.h"

typedef struct {
    GtkWidget *window;
    struct {
        GtkWidget* vehicleSpeed;
        GtkWidget* coolantTemperature;
        GtkWidget* engineSpeed;
    } data;
    struct {
        GtkListBox* listView;
        GtkEntry* input;
        GtkButton* inputButton;
        GtkCheckButton* milOn;
        GtkCheckButton* dtcCleared;
        GtkWidget *invalidDtc;
    } dtcs;
} ELM327SimGui;

void elm327_sim_cli_display_help();
int elm327_sim_cli_main(int argc, char **argv);

#endif
