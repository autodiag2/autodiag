#include "libautodiag/jni/tmp_file.h"

#ifdef OS_ANDROID

static char * tmpDir = null;

JNIEXPORT void JNICALL Java_com_autodiag_elm327emu_libautodiag_setTmpDir(JNIEnv *env, jobject thiz, jstring path) {
    const char *nativePath = (*env)->GetStringUTFChars(env, path, null);
    tmpDir = strdup(nativePath);
    (*env)->ReleaseStringUTFChars(env, path, nativePath);}

char * jni_tmp_dir_get() {
    return tmpDir;
}

#endif