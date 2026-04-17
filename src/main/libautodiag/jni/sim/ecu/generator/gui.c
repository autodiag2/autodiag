#include "libautodiag/jni/sim/ecu/generator/gui.h"

#ifdef OS_ANDROID

    static JavaVM *g_vm;
    static jclass g_libautodiag;
    static jmethodID mid_signal_value;
    static jmethodID mid_mil_status;
    static jmethodID mid_dtc_cleared;
    static jmethodID mid_ecu_name;
    static jmethodID mid_vin;
    static jmethodID mid_dtcs;
    static jmethodID mid_set_dtc_cleared;
    static jmethodID mid_signal_value;

    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
        JNIEnv *env;
        g_vm = vm;

        if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK)
            return JNI_ERR;

        jclass cls = (*env)->FindClass(env, "com/github/autodiag2/elm327emu/libautodiag");
        g_libautodiag = (*env)->NewGlobalRef(env, cls);
        mid_signal_value = (*env)->GetStaticMethodID(env, g_libautodiag, "getSignalValue", "(Ljava/lang/String;)D");
        mid_mil_status   = (*env)->GetStaticMethodID(env, g_libautodiag, "getMil", "()Z");
        mid_dtc_cleared   = (*env)->GetStaticMethodID(env, g_libautodiag, "getDtcCleared", "()Z");
        mid_ecu_name   = (*env)->GetStaticMethodID(env, g_libautodiag, "getEcuName", "()Ljava/lang/String;");
        mid_vin   = (*env)->GetStaticMethodID(env, g_libautodiag, "getVin", "()Ljava/lang/String;");
        mid_dtcs = (*env)->GetStaticMethodID(env, g_libautodiag, "getDtcs", "()[Ljava/lang/String;");
        mid_set_dtc_cleared = (*env)->GetStaticMethodID(
            env,
            g_libautodiag,
            "setDtcCleared",
            "(Z)V"
        );
        ad_object_vehicle_signal_register_all();
        return JNI_VERSION_1_6;
    }

    static JNIEnv *get_env(void) {
        JNIEnv *env = null;
        if ((*g_vm)->GetEnv(g_vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
            (*g_vm)->AttachCurrentThread(g_vm, &env, null);
        }
        return env;
    }

    #define response_saej1979_pid_with_signal(signal_path) { \
        ad_object_vehicle_signal * signal = ad_signal_get(signal_path); \
        jstring signal_path_j = (*env)->NewStringUTF(env, signal_path); \
        double value = (*env)->CallStaticDoubleMethod(env, g_libautodiag, mid_signal_value, signal_path_j); \
        (*env)->DeleteLocalRef(env, signal_path_j); \
        if ( value != NAN ) { \
            Buffer * signal_inverted = ad_expr_reduce_invert(value, signal->rv_formula, null); \
            ad_buffer_append_melt(binResponse, signal_inverted); \
        } \
    }

    static Buffer * response_saej1979_pid(SimECUGenerator *generator, final byte pid, int frameNumber) {
        unsigned * seed = generator->context;
        Buffer * binResponse = ad_buffer_new();
        JNIEnv *env = get_env();
        final SimECUGenerator * parent = (SimECUGenerator*) generator->state;
        bool generic = true;
        bool mil_status   = (*env)->CallStaticBooleanMethod(env, g_libautodiag, mid_mil_status);
        bool dtc_cleared   = (*env)->CallStaticBooleanMethod(env, g_libautodiag, mid_dtc_cleared);
        jobjectArray dtcs = (jobjectArray)(*env)->CallStaticObjectMethod(env, g_libautodiag, mid_dtcs);
        jsize dtc_count = (*env)->GetArrayLength(env, dtcs);
        // Should append only bytes according to the PID, but for simplicity we just append random data
        switch(pid) {
            case 0xC0:
            case 0xA0:
            case 0x80:
            case 0x60:
            case 0x40:
            case 0x20:
            case 0x00: {
                ad_buffer_append_melt(binResponse, ad_buffer_from_ascii_hex("FFFFFFFF"));
                generic = false;
            } break;
            case 0x01: {
                bool is_checked = mil_status;
                Buffer* status = ad_buffer_new();
                ad_buffer_pad(status, 4, 0x00);
                if ( ! dtc_cleared ) {
                    status->buffer[0] = dtc_count;
                    status->buffer[0] |= is_checked << 7;
                }
                ad_buffer_append_melt(binResponse, status);
                generic = false;
            } break;
        }
        if ( generic ) {
            ad_object_vehicle_signal * signal = ad_signal_get_from_saej1979_pid(pid);
            if ( signal == null ) {
                return parent->response_saej1979_pid(parent, pid, frameNumber);
            }
            response_saej1979_pid_with_signal(ad_object_vehicle_signal_get_exec_path(signal));
        }
        return binResponse;
    }
    static Buffer * response_saej1979_dtcs(SimECUGenerator *generator, int service_id) {
        JNIEnv *env = get_env();
        Buffer * binResponse = ad_buffer_new();
        bool dtc_cleared   = (*env)->CallStaticBooleanMethod(env, g_libautodiag, mid_dtc_cleared);
        jobjectArray dtcs = (jobjectArray)(*env)->CallStaticObjectMethod(env, g_libautodiag, mid_dtcs);
        jsize dtc_count = (*env)->GetArrayLength(env, dtcs);
        ad_list_Buffer * dtcs_list = ad_list_Buffer_new();
        if ( service_id == OBD_SERVICE_SHOW_DTC ) {
            if ( ! dtc_cleared ) {
                for (jsize i = 0; i < dtc_count; i++) {
                    jstring s = (jstring)(*env)->GetObjectArrayElement(env, dtcs, i);
                    const char *dtc = (*env)->GetStringUTFChars(env, s, 0);

                    Buffer *dtc_bin = saej1979_dtc_bin_from_string((char*)dtc);
                    if ( dtc_bin == null ) {
                        log_msg(LOG_ERROR, "invalid dtc found");
                    } else {
                        ad_list_Buffer_append(dtcs_list, dtc_bin);
                    }
                    
                    (*env)->ReleaseStringUTFChars(env, s, dtc);
                    (*env)->DeleteLocalRef(env, s);
                }
            }
            for(int i = 0; i < dtcs_list->size; i++) {
                ad_buffer_append_melt(binResponse, dtcs_list->list[i]);
                dtcs_list->list[i] = null;
            }
            ad_list_Buffer_free(dtcs_list);
        }
        return binResponse;
    }
    static Buffer * response_saej1979_vehicle_identification_request_info_type(SimECUGenerator * generator, byte infoType) {
        JNIEnv *env = get_env();
        unsigned * seed = generator->context;
        jstring ecu_name_j = (*env)->CallStaticObjectMethod(env, g_libautodiag, mid_ecu_name);
        jstring vin_j = (*env)->CallStaticObjectMethod(env, g_libautodiag, mid_vin);

        const char *ecu_name = (*env)->GetStringUTFChars(env, ecu_name_j, 0);
        const char *vin = (*env)->GetStringUTFChars(env, vin_j, 0);
        switch(infoType) {
            case 0x00:                                          return ad_buffer_from_ascii_hex("FFFFFFFF");
            case 0x01:                                          return ad_buffer_from_ascii_hex("05");
            case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_VIN:   return ad_buffer_from_ascii(vin);
            case 0x03:                                          return ad_buffer_from_ascii_hex("01");
            case 0x04:                                          return ad_buffer_new_random_with_seed(16, seed);
            case 0x05:                                          return ad_buffer_from_ascii_hex("01");
            case 0x06:                                          return ad_buffer_new_random_with_seed(4, seed);
            case 0x07:                                          return ad_buffer_from_ascii_hex("01");
            case 0x08:                                          return ad_buffer_new_random_with_seed(4, seed);
            case 0x09:                                          return ad_buffer_from_ascii_hex("01");
            case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: {
                final Buffer * name = ad_buffer_from_ascii(ecu_name);
                ad_buffer_pad(name, 20, 0x00);
                return name;
            } break;
        }
        return ad_buffer_new();
    }
    static Buffer * response(SimECUGenerator *generator, final Buffer *binRequest) {
        JNIEnv *env = get_env();
        if ( binRequest->size == 0 ) {
            return ad_buffer_new();
        }

        final SimECUGenerator * parent = (SimECUGenerator*) generator->state;
        final Buffer *binResponse = ad_buffer_new();
        bool useParent = true;
        if ( ! sim_ecu_generator_fill_success(binResponse, binRequest) ) {
            return ad_buffer_new();
        }
        switch(binRequest->buffer[0]) {
            case OBD_SERVICE_CLEAR_DTC: {
                (*env)->CallStaticVoidMethod(
                    env,
                    g_libautodiag,
                    mid_set_dtc_cleared,
                    JNI_TRUE
                );
                log_msg(LOG_DEBUG, "Clearing DTCs");
                useParent = false;
            } break;
            case OBD_SERVICE_SHOW_DTC: {
                binResponse = generator->response_saej1979_dtcs_wrapper(generator, binRequest->buffer[0]);
                useParent = false;
            } break;
            case OBD_SERVICE_SHOW_CURRENT_DATA:
            case OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA:
                binResponse = generator->response_saej1979_pids(generator, binRequest);
                useParent = false;
                break;
            case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION:
                binResponse = generator->response_saej1979_vehicle_identification_request(generator, binRequest);
                useParent = false;
                break;
        }
        if ( useParent ) {
            return parent->response(parent, binRequest);
        }
        return binResponse;
    }
    static char * context_to_string(SimECUGenerator * this) {
        return strdup("");
    }
    static bool context_load_from_string(SimECUGenerator * this, char * context) {
        return true;
    }
    SimECUGenerator* sim_ecu_generator_new_gui() {
        SimECUGenerator * generator = sim_ecu_generator_new();
        generator->response = SIM_ECU_GENERATOR_RESPONSE(response);
        generator->context_load_from_string = SIM_ECU_GENERATOR_CONTEXT_LOAD_FROM_STRING(context_load_from_string);
        generator->context_to_string = SIM_ECU_GENERATOR_CONTEXT_TO_STRING(context_to_string);
        generator->type = strdup("GUI");
        generator->flavour.is_Iso15765_4 = false;
        generator->response_saej1979_pid = response_saej1979_pid;
        generator->response_saej1979_dtcs = response_saej1979_dtcs;
        generator->response_saej1979_vehicle_identification_request_info_type = response_saej1979_vehicle_identification_request_info_type;
        generator->state = sim_ecu_generator_new_citroen_c5_x7();
        generator->context = null;
        return generator;
    }

#endif