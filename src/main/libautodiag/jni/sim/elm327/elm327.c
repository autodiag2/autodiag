#include "libautodiag/jni/sim/elm327/elm327.h"
#ifdef OS_ANDROID

JNIEXPORT jstring JNICALL Java_com_autodiag_elm327emu_libautodiag_launchEmu(JNIEnv *env, jobject thiz) {
    log_set_level(LOG_DEBUG);
    SimELM327 *elm327 = sim_elm327_new();
    if (!elm327) {
        return null;
    }
    elm327->device_type = strdup("loopback");

    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);

    if (!elm327->device_location) {
        return null;
    }

    return (*env)->NewStringUTF(env, elm327->device_location);
}

#endif