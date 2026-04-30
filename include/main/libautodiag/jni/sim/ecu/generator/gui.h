#ifndef __AD_JNI_SIM_ECU_GENERATOR_GUI_H
#define __AD_JNI_SIM_ECU_GENERATOR_GUI_H

#include "libautodiag/jni/loader.h"
#include "libautodiag/compile_target.h"
#include "libautodiag/sim/ecu/generator.h"
#include "libautodiag/com/vehicle_signal.h"

#ifdef OS_ANDROID
#include <jni.h>
#include <string.h>

SimECUGenerator* sim_ecu_generator_new_gui();

#endif

#endif