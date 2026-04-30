#ifndef __AD_JNI_LOADER_H
#define __AD_JNI_LOADER_H

#include "libautodiag/compile_target.h"
#include "libautodiag/lang/all.h"
#include "libautodiag/com/vehicle_signal.h"

#ifdef OS_ANDROID
#   include <jni.h>
#   include <string.h>
    JNIEnv *get_env();
    JavaVM* getJavaVM();
    extern jclass g_libautodiag;
    extern jmethodID mid_signal_value;
    extern jmethodID mid_mil_status;
    extern jmethodID mid_dtc_cleared;
    extern jmethodID mid_ecu_name;
    extern jmethodID mid_vin;
    extern jmethodID mid_dtcs;
    extern jmethodID mid_set_dtc_cleared;
    extern jmethodID mid_signal_value;
#endif

#endif