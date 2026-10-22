#include "libautodiag/jni/log.h"

#ifdef OS_ANDROID

    JNIEXPORT void JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_setLogLevel(JNIEnv *env, jobject thiz, jstring level) {
        const char *levelStr = (*env)->GetStringUTFChars(env, level, null);
        log_set_level(log_level_from_str((char*)levelStr));
        (*env)->ReleaseStringUTFChars(env, level, levelStr);
    }

#endif