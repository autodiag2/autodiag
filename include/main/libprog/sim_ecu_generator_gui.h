#ifndef __SIM_OBD_GENERATORS_H
#define __SIM_OBD_GENERATORS_H

#include "libautodiag/sim/elm327/elm327.h"
#include "libautodiag/sim/elm327/sim_generators.h"

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

SimECUGenerator* sim_ecu_generator_new_gui();

#endif