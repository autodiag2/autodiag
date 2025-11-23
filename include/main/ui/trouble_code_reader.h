#ifndef __TROUBLE_CODE_READER_H
#define __TROUBLE_CODE_READER_H

#define MODULE_CODES_READER "Trouble Code Reader"

#include "main.h"
#include "ui.h"
#include "libautodiag/com/obd/obd.h"
#include "ui/widget/menubar.h"

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
        GtkCheckMenuItem *filtered;
        struct {
            struct {
                GtkCheckMenuItem *all;
                GtkWidget *filter_by;
                GtkWidget *filter_by_menu;
            } source;
        } data;
    } menuBar;
} TroubleCodeReaderGui;

void module_init_read_codes(GtkBuilder *builder);

#define trouble_code_reader_error_feedback_obd(iface) \
    error_feedback_obd(gui->errorFeedback,iface,list_serial_get_selected())

#endif
