#include "libautodiag/jni/loader.h"

#ifdef OS_ANDROID
    #include <pthread.h>

    static JavaVM *g_vm;
    static pthread_key_t g_attached_thread_key;
    static bool g_attached_thread_key_ready;
    jclass g_libautodiag;
    jmethodID mid_signal_value;
    jmethodID mid_mil_status;
    jmethodID mid_dtc_cleared;
    jmethodID mid_ecu_name;
    jmethodID mid_vin;
    jmethodID mid_dtcs;
    jmethodID mid_set_dtc_cleared;

    JavaVM *getJavaVM(void) {
        return g_vm;
    }

    static void detach_current_thread(void *value) {
        JavaVM *vm = value;
        if (vm != null) {
            (*vm)->DetachCurrentThread(vm);
        }
    }

    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
        JNIEnv *env;
        g_vm = vm;

        if (pthread_key_create(&g_attached_thread_key, detach_current_thread) != 0) {
            g_vm = null;
            return JNI_ERR;
        }
        g_attached_thread_key_ready = true;

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

    JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
        JNIEnv *env = null;
        if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) == JNI_OK) {
            if (g_libautodiag != null) {
                (*env)->DeleteGlobalRef(env, g_libautodiag);
                g_libautodiag = null;
            }
        }
        if (g_attached_thread_key_ready) {
            pthread_key_delete(g_attached_thread_key);
            g_attached_thread_key_ready = false;
        }
        g_vm = null;
    }

    JNIEnv *get_env(void) {
        JNIEnv *env = null;
        if (g_vm == null || !g_attached_thread_key_ready) {
            return null;
        }
        if ((*g_vm)->GetEnv(g_vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
            if ((*g_vm)->AttachCurrentThread(g_vm, &env, null) != JNI_OK) {
                return null;
            }
            if (pthread_setspecific(g_attached_thread_key, g_vm) != 0) {
                (*g_vm)->DetachCurrentThread(g_vm);
                return null;
            }
        }
        return env;
    }
#endif