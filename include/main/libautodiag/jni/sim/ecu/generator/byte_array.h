#ifndef __AD_JNI_SIM_ECU_GENERATOR_BYTE_ARRAY_H
#define __AD_JNI_SIM_ECU_GENERATOR_BYTE_ARRAY_H

#include "libautodiag/jni/loader.h"
#include "libautodiag/compile_target.h"
#include "libautodiag/sim/ecu/generator.h"
#include "libautodiag/com/vehicle_signal.h"
#include "libautodiag/buffer.h"

#ifdef OS_ANDROID
#include <jni.h>
#include <string.h>

typedef struct {
    JavaVM *vm;
    jobject callback;   // global ref
    jmethodID method;
} JavaCallbackContext;

SimECUGenerator* sim_ecu_generator_new_byte_array(jobject callbackObject);

#endif

#endif