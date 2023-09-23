#ifndef __TROUBLE_CODE_READER_H
#define __TROUBLE_CODE_READER_H

#define MODULE_CODES_READER "Trouble Code Reader"

#define _GNU_SOURCE
#include <stdio.h>
#include "gtk.h"
#include "log.h"
#include "thread.h"
#include "main.h"
#include "error_feedback.h"
#include "com/serial/obd/saej1979/saej1979.h"

typedef struct {
    GtkWidget *window, *noObdInterface;
    GtkWidget *clearConfirm;
    GtkSpinner *actionWaitIcon;
    GtkWidget *noObdData;
    struct {
        GtkToggleButton *stored;
        GtkToggleButton *pending;
        GtkToggleButton *permanent;
    } read;
    struct {
        GtkMessageDialog *window;
    } serialError;
    struct {
        GtkLabel* text;
        GtkImage* iconOn;
        GtkImage* iconOff;
    } mil;
    struct {
        GtkLabel* count;
        GtkListBox* list;
        GtkTextView * description;
        GtkTextBuffer *descriptionText;
        GtkTextView * causeSolution;
        GtkTextBuffer*causeSolutionText;
        GtkTextView * explanation;
        GtkTextBuffer*explanationText;
    } dtc;
} TroubleCodeReaderGui;

void module_init_read_codes(GtkBuilder *builder);

#endif
