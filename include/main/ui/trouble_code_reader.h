#ifndef __TROUBLE_CODE_READER_H
#define __TROUBLE_CODE_READER_H

#define MODULE_CODES_READER "Trouble Code Reader"

#include "main.h"
#include "ui.h"
#include "com/obd/obd.h"

typedef struct {
    GtkWidget *window;
    GtkSpinner *actionWaitIcon;
    GtkWidget *noObdData;
    ErrorFeedbackWindows errorFeedback;
    struct {
        GtkToggleButton *stored;
        GtkToggleButton *pending;
        GtkToggleButton *permanent;
        ThreadedAction;
    } read;
    struct {
        GtkWidget *confirm;
        ThreadedAction;
    } clear;
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
    struct {
        struct {
            GtkWidget       *window;
            GtkTextView     *textView;
            GtkTextBuffer   *text;
            ThreadedAction;            
        } showECUsBuffer;
    } menuBar;
} TroubleCodeReaderGui;

void module_init_read_codes(GtkBuilder *builder);

#define trouble_code_reader_error_feedback_obd(iface) \
    error_feedback_obd(tcgui->errorFeedback,iface,serial_list_get_selected())

#endif
