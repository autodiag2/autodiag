#include "libautodiag/jni/com/vehicle_signal.h"

#ifdef OS_ANDROID

jobject jni_create_sim_signal(JNIEnv *env, ad_object_vehicle_signal *signal) {
    static jclass sim_signal_cls = NULL;
    static jmethodID sim_signal_ctor = NULL;

    if (sim_signal_cls == NULL) {
        jclass local = (*env)->FindClass(
            env,
            "com/github/autodiag2/elm327emu/SimSignal"
        );
        if (local == NULL) {
            return NULL;
        }

        sim_signal_cls = (*env)->NewGlobalRef(env, local);
        (*env)->DeleteLocalRef(env, local);

        sim_signal_ctor = (*env)->GetMethodID(
            env,
            sim_signal_cls,
            "<init>",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;DDDLjava/lang/String;)V"
        );
        if (sim_signal_ctor == NULL) {
            return NULL;
        }
    }

    const char *path = ad_object_vehicle_signal_get_exec_path(signal);
    const char *name = signal->name != NULL ? signal->name : "";
    const char *unit = signal->unit != NULL ? signal->unit : "";
    const char *category = signal->category != NULL ? signal->category : "";
    double min = signal->rv_min;
    double max = signal->rv_max;
    double step = 1.0;

    jstring path_j = (*env)->NewStringUTF(env, path != NULL ? path : "");
    jstring name_j = (*env)->NewStringUTF(env, name);
    jstring unit_j = (*env)->NewStringUTF(env, unit);
    jstring category_j = (*env)->NewStringUTF(env, category);

    jobject obj = (*env)->NewObject(
        env,
        sim_signal_cls,
        sim_signal_ctor,
        path_j,
        name_j,
        unit_j,
        min,
        max,
        step,
        category_j
    );

    (*env)->DeleteLocalRef(env, path_j);
    (*env)->DeleteLocalRef(env, name_j);
    (*env)->DeleteLocalRef(env, unit_j);
    (*env)->DeleteLocalRef(env, category_j);

    return obj;
}

JNIEXPORT jobjectArray JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_getSimSignals(JNIEnv *env, jclass clazz) {
    jclass sim_signal_cls = (*env)->FindClass(env, "com/github/autodiag2/elm327emu/SimSignal");
    if (sim_signal_cls == null) {
        return null;
    }

    jmethodID sim_signal_ctor = (*env)->GetMethodID(
        env,
        sim_signal_cls,
        "<init>",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;DDDLjava/lang/String;)V"
    );
    if (sim_signal_ctor == null) {
        (*env)->DeleteLocalRef(env, sim_signal_cls);
        return null;
    }

    ad_object_hashmap_string_vehicle_signal * signals = ad_signals_get();
    if (signals == null) {
        jobjectArray out = (*env)->NewObjectArray(env, 0, sim_signal_cls, null);
        (*env)->DeleteLocalRef(env, sim_signal_cls);
        return out;
    }

    jobjectArray out = (*env)->NewObjectArray(env, signals->size, sim_signal_cls, null);
    if (out == null) {
        (*env)->DeleteLocalRef(env, sim_signal_cls);
        return null;
    }

    for (int i = 0; i < signals->size; i++) {
        jobject sim_signal = jni_create_sim_signal(env, signals->values[i]);
        (*env)->SetObjectArrayElement(env, out, i, sim_signal);
        (*env)->DeleteLocalRef(env, sim_signal);
    }
    (*env)->DeleteLocalRef(env, sim_signal_cls);
    return out;
}

#endif