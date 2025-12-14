#ifndef __JNI_SIM_ECU_GENERATOR_GUI_H
#define __JNI_SIM_ECU_GENERATOR_GUI_H

#include "libautodiag/compile_target.h"
#include "libautodiag/sim/ecu/generator.h"

#ifdef OS_ANDROID
#include <jni.h>
#include <string.h>

SimECUGenerator* sim_ecu_generator_new_gui();

#endif

#endif