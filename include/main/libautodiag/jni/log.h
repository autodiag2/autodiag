#ifndef __AD_JNI_LOADER_H
#define __AD_JNI_LOADER_H

#include "libautodiag/compile_target.h"
#include "libautodiag/lang/all.h"
#include "libautodiag/log.h"

#ifdef OS_ANDROID
#   include <jni.h>
#   include <string.h>
    JNIEXPORT void JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_setLogLevel(JNIEnv *env, jobject thiz, jstring level);
#endif

#endif