#ifndef __JNI_TMP_FILE_H
#define __JNI_TMP_FILE_H

#include "libautodiag/compile_target.h"
#include "libautodiag/lang/all.h"

#ifdef OS_ANDROID
#include <jni.h>

char * jni_tmp_dir_get();
void jni_tmp_dir_set(char *path);

#endif

#endif