#ifndef __JNI_TMP_FILE_H
#define __JNI_TMP_FILE_H

#include "libautodiag/compile_target.h"
#include "libautodiag/lang/all.h"

#ifdef OS_ANDROID
#include <jni.h>

JNIEXPORT void JNICALL Java_com_autodiag_elm327emu_libautodiag_setTmpDir(JNIEnv *env, jobject thiz, jstring path);
char * jni_tmp_dir_get();

#endif

#endif