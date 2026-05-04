#include "libautodiag/jni/loader.h"

#ifdef OS_ANDROID
    static JavaVM *g_vm;
    jclass g_libautodiag;
    jmethodID mid_signal_value;
    jmethodID mid_mil_status;
    jmethodID mid_dtc_cleared;
    jmethodID mid_ecu_name;
    jmethodID mid_vin;
    jmethodID mid_dtcs;
    jmethodID mid_set_dtc_cleared;

    JavaVM* getJavaVM() {
        return g_vm;
    }

    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
        JNIEnv *env;
        g_vm = vm;

        if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK)
            return JNI_ERR;

        jclass cls = (*env)->FindClass(env, "com/github/autodiag2/elm327emu/libautodiag");
        g_libautodiag = (*env)->NewGlobalRef(env, cls);
        mid_signal_value = (*env)->GetStaticMethodID(env, g_libautodiag, "getSignalValue", "(BLjava/lang/String;)D");
        mid_mil_status   = (*env)->GetStaticMethodID(env, g_libautodiag, "getMil", "(B)Z");
        mid_dtc_cleared   = (*env)->GetStaticMethodID(env, g_libautodiag, "getDtcCleared", "(B)Z");
        mid_ecu_name   = (*env)->GetStaticMethodID(env, g_libautodiag, "getEcuName", "(B)Ljava/lang/String;");
        mid_vin   = (*env)->GetStaticMethodID(env, g_libautodiag, "getVin", "(B)Ljava/lang/String;");
        mid_dtcs = (*env)->GetStaticMethodID(env, g_libautodiag, "getDtcs", "(B)[Ljava/lang/String;");
        mid_set_dtc_cleared = (*env)->GetStaticMethodID(
            env,
            g_libautodiag,
            "setDtcCleared",
            "(BZ)V"
        );
        ad_object_vehicle_signal_register_all();
        return JNI_VERSION_1_6;
    }

    JNIEnv *get_env() {
        JNIEnv *env = null;
        if ((*g_vm)->GetEnv(g_vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
            (*g_vm)->AttachCurrentThread(g_vm, &env, null);
        }
        return env;
    }
#endif