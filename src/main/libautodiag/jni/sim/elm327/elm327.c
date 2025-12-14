#include "libautodiag/jni/sim/elm327/elm327.h"
#include "libautodiag/jni/sim/ecu/generator/gui.h"

#ifdef OS_ANDROID

JNIEXPORT jstring JNICALL Java_com_autodiag_elm327emu_libautodiag_launchEmu(JNIEnv *env, jobject thiz) {
    log_set_level(LOG_DEBUG);
    SimELM327 *sim = sim_elm327_new();
    if (!sim) {
        return null;
    }
    sim->device_type = strdup("socket");

    assert(0 < LIST_SIM_ECU(sim->ecus)->size);
    final SimECU * target_ecu = LIST_SIM_ECU(sim->ecus)->list[LIST_SIM_ECU(sim->ecus)->size - 1];
    final SimECUGenerator * generator;
    target_ecu->generator = sim_ecu_generator_new_gui();

    sim_elm327_loop_as_daemon(sim);
    sim_elm327_loop_daemon_wait_ready(sim);

    if (!sim->device_location) {
        return null;
    }

    return (*env)->NewStringUTF(env, sim->device_location);
}

#endif