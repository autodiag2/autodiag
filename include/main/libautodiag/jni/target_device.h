#ifndef __AD_JNI_TARGET_DEVICE_H
#define __AD_JNI_TARGET_DEVICE_H

#include "libautodiag/compile_target.h"
#include "libautodiag/lang/all.h"
#include "libautodiag/jni/loader.h"

#ifdef OS_ANDROID
#include <jni.h>

char * jni_data_dir_get();
void jni_data_dir_set(char *path);

#endif

#endif