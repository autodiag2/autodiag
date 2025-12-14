#include "libautodiag/jni/sim/ecu/generator/gui.h"

#ifdef OS_ANDROID

    static JavaVM *g_vm;
    static jclass g_libautodiag;
    static jmethodID mid_speed;
    static jmethodID mid_temp;
    static jmethodID mid_rpm;
    static jmethodID mid_mil;
    static jmethodID mid_dtc;
    static jmethodID mid_ecu;
    static jmethodID mid_vin;

    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
        JNIEnv *env;
        g_vm = vm;

        if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK)
            return JNI_ERR;

        jclass cls = (*env)->FindClass(env, "com/autodiag/elm327emu/libautodiag");
        g_libautodiag = (*env)->NewGlobalRef(env, cls);

        mid_speed = (*env)->GetStaticMethodID(env, g_libautodiag, "getVehicleSpeed", "()I");
        mid_temp  = (*env)->GetStaticMethodID(env, g_libautodiag, "getCoolantTemp", "()I");
        mid_rpm   = (*env)->GetStaticMethodID(env, g_libautodiag, "getEngineRpm", "()I");
        mid_mil   = (*env)->GetStaticMethodID(env, g_libautodiag, "getMil", "()Z");
        mid_dtc   = (*env)->GetStaticMethodID(env, g_libautodiag, "getDtcCleared", "()Z");
        mid_ecu   = (*env)->GetStaticMethodID(env, g_libautodiag, "getEcuName", "()Ljava/lang/String;");
        mid_vin   = (*env)->GetStaticMethodID(env, g_libautodiag, "getVin", "()Ljava/lang/String;");

        return JNI_VERSION_1_6;
    }

    static JNIEnv *get_env(void) {
        JNIEnv *env = null;
        if ((*g_vm)->GetEnv(g_vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
            (*g_vm)->AttachCurrentThread(g_vm, &env, null);
        }
        return env;
    }

    static Buffer * response(SimECUGenerator *generator, final Buffer *binRequest) {
        JNIEnv *env = get_env();

        int speed = (*env)->CallStaticIntMethod(env, g_libautodiag, mid_speed);
        int temp  = (*env)->CallStaticIntMethod(env, g_libautodiag, mid_temp);
        int rpm   = (*env)->CallStaticIntMethod(env, g_libautodiag, mid_rpm);
        int mil   = (*env)->CallStaticBooleanMethod(env, g_libautodiag, mid_mil);
        int dtc   = (*env)->CallStaticBooleanMethod(env, g_libautodiag, mid_dtc);

        jstring ecu_j = (*env)->CallStaticObjectMethod(env, g_libautodiag, mid_ecu);
        jstring vin_j = (*env)->CallStaticObjectMethod(env, g_libautodiag, mid_vin);

        const char *ecu = (*env)->GetStringUTFChars(env, ecu_j, 0);
        const char *vin = (*env)->GetStringUTFChars(env, vin_j, 0);

        final Buffer *binResponse = buffer_new();
        sim_ecu_generator_fill_success(binResponse, binRequest);
        switch(binRequest->buffer[0]) {
            case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION: {
                if ( 1 < binRequest->size ) {            
                    switch(binRequest->buffer[1]) {
                        case 0x00: {
                            buffer_append(binResponse, buffer_from_ascii_hex("FFFFFFFFFF"));
                            break;
                        }
                        case 0x01: {
                            buffer_append_byte(binResponse, 0x05);
                            break;
                        }
                        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_VIN: {
                            buffer_append(binResponse,buffer_from_ascii(vin));                
                            break;
                        }
                        case 0x03: {
                            buffer_append_byte(binResponse,0x01);
                            break;
                        }
                        case 0x04: {
                            buffer_append(binResponse,buffer_new_random(16));                
                            break;
                        }
                        case 0x05: {
                            buffer_append_byte(binResponse,0x01);
                            break;
                        }
                        case 0x06: {
                            buffer_append(binResponse,buffer_new_random(4));
                            break;
                        }
                        case 0x07: {
                            buffer_append_byte(binResponse,0x01);
                            break;
                        }
                        case 0x08: {
                            buffer_append(binResponse,buffer_new_random(4));
                            break;
                        }
                        case 0x09: {
                            buffer_append_byte(binResponse,0x01);
                            break;
                        }
                        case OBD_SERVICE_REQUEST_VEHICLE_INFORMATION_ECU_NAME: {
                            final Buffer * name = buffer_from_ascii(ecu);
                            buffer_padding(name, 20, 0x00);
                            buffer_append(binResponse, name);
                            break;
                        }
                        case 0x0B: {
                            buffer_append(binResponse,buffer_new_random(4));
                            break;
                        }
                    }
                } else {
                    log_msg(LOG_ERROR, "Missing PID");
                }
            } break;
        }
        (*env)->ReleaseStringUTFChars(env, ecu_j, ecu);
        (*env)->ReleaseStringUTFChars(env, vin_j, vin);
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
        generator->state = null;
        generator->context = null;
        return generator;
    }

#endif