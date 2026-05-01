#include "libautodiag/jni/sim/elm327/elm327.h"
#include "libautodiag/jni/sim/ecu/generator/gui.h"
#include "libautodiag/jni/target_device.h"
#include "libautodiag/jni/sim/ecu/generator/byte_array.h"

#ifdef OS_ANDROID
static SimELM327 *_sim = null;
SimELM327* jni_sim_elm327_get() {
    if ( _sim == null ) {
        _sim = sim_elm327_new();
    }
    assert(_sim != null);
    return _sim;
}
JNIEXPORT jstring JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_launchEmu(JNIEnv *env, jobject thiz, jstring path, jstring kind) {
    log_set_level(LOG_DEBUG);
    
    const char *nativePath = (*env)->GetStringUTFChars(env, path, null);
    jni_data_dir_set(strdup(nativePath));
    (*env)->ReleaseStringUTFChars(env, path, nativePath);

    const char *kindStr = (*env)->GetStringUTFChars(env, kind, null);

    SimELM327 *sim = jni_sim_elm327_get();
    sim->device_type = sim_elm327_device_type_from_str((char*)kindStr);

    (*env)->ReleaseStringUTFChars(env, kind, kindStr);

    sim_elm327_loop_as_daemon(sim);
    sim_elm327_loop_daemon_wait_ready(sim);

    if (!sim->device_location) {
        return null;
    }

    return (*env)->NewStringUTF(env, sim->device_location);
}
JNIEXPORT jobjectArray JNICALL
Java_com_github_autodiag2_elm327emu_libautodiag_getProtocols(
    JNIEnv *env,
    jobject thiz
) {
    const int first = ELM327_PROTO_SAE_J1850_1;
    const int last  = ELM327_PROTO_USER2_CAN;
    const int count = last - first + 1;

    jclass stringClass = (*env)->FindClass(env, "java/lang/String");
    if (!stringClass)
        return NULL;

    jobjectArray array = (*env)->NewObjectArray(env, count, stringClass, NULL);
    if (!array)
        return NULL;

    int idx = 0;
    for (int i = first; i <= last; i++, idx++) {
        const char *proto = elm327_protocol_to_string(i);
        if (!proto)
            proto = "";

        jstring jproto = (*env)->NewStringUTF(env, proto);
        (*env)->SetObjectArrayElement(env, array, idx, jproto);
        (*env)->DeleteLocalRef(env, jproto);
    }

    return array;
}
JNIEXPORT void JNICALL
Java_com_github_autodiag2_elm327emu_libautodiag_setProtocol(
    JNIEnv *env,
    jobject thiz,
    jint protocol
) {
    SimELM327 *sim = jni_sim_elm327_get();
    if (!sim)
        return;

    sim->default_protocol = protocol;
    sim->protocolRunning = protocol;
    sim->nvm.protocol = protocol;
}
JNIEXPORT jint JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_getProtocol(JNIEnv *env, jobject thiz) {
    SimELM327 *sim = jni_sim_elm327_get();
    if (!sim)
        return -1;
    return sim->protocolRunning;
}

JNIEXPORT void JNICALL
Java_com_github_autodiag2_elm327emu_libautodiag_setResponseByteArrayByAddress(
    JNIEnv *env,
    jobject thiz,
    jbyte address,
    jobject callbackObject
) {
    SimELM327 *sim = jni_sim_elm327_get();
    if (!sim) return;

    SimECU *ecu = ad_list_SimECU_search_by_address(sim->ecus, (byte)address);

    if (ecu == NULL) {
        ecu = sim_ecu_new((byte)address);
        ad_list_SimECU_append(sim->ecus, ecu);
    }

    // create new byte array generator (JNI callback is handled inside)
    ecu->generator = sim_ecu_generator_new_byte_array(callbackObject);
}
JNIEXPORT void JNICALL
Java_com_github_autodiag2_elm327emu_libautodiag_setResponseGuiByAddress(
    JNIEnv *env,
    jobject thiz,
    jbyte address
) {
    SimELM327 *sim = jni_sim_elm327_get();
    if (!sim) return;

    SimECU *ecu = ad_list_SimECU_search_by_address(sim->ecus, (byte)address);

    if (ecu == NULL) {
        ecu = sim_ecu_new((byte)address);
        ad_list_SimECU_append(sim->ecus, ecu);
    }

    ecu->generator = sim_ecu_generator_new_gui();
}
#endif