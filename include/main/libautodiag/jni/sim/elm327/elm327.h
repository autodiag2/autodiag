#ifndef __JNI_SIM_ELM327_H
#define __JNI_SIM_ELM327_H

#include "libautodiag/compile_target.h"

#ifdef OS_ANDROID
#include <jni.h>
#include <string.h>
#include "libautodiag/sim/elm327/elm327.h"

JNIEXPORT jstring JNICALL Java_com_autodiag_elm327emu_libautodiag_launchEmu(JNIEnv *env, jobject thiz);


#endif
#endif