#include "libautodiag/jni/sim/ecu/generator/gui.h"

#ifdef OS_ANDROID

    static JavaVM *g_vm;
    static jclass g_libautodiag;
    static jmethodID mid_vehicle_speed;
    static jmethodID mid_coolant_temperature;
    static jmethodID mid_engine_rpm;
    static jmethodID mid_mil_status;
    static jmethodID mid_dtc_cleared;
    static jmethodID mid_ecu_name;
    static jmethodID mid_vin;
    static jmethodID mid_dtcs;
    static jmethodID mid_set_dtc_cleared;

    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
        JNIEnv *env;
        g_vm = vm;

        if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK)
            return JNI_ERR;

        jclass cls = (*env)->FindClass(env, "com/autodiag/elm327emu/libautodiag");
        g_libautodiag = (*env)->NewGlobalRef(env, cls);

        mid_vehicle_speed = (*env)->GetStaticMethodID(env, g_libautodiag, "getVehicleSpeed", "()I");
        mid_coolant_temperature  = (*env)->GetStaticMethodID(env, g_libautodiag, "getCoolantTemp", "()I");
        mid_engine_rpm   = (*env)->GetStaticMethodID(env, g_libautodiag, "getEngineRpm", "()I");
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

        int vehicle_speed = (*env)->CallStaticIntMethod(env, g_libautodiag, mid_vehicle_speed);
        int coolant_temperature  = (*env)->CallStaticIntMethod(env, g_libautodiag, mid_coolant_temperature);
        int engine_rpm   = (*env)->CallStaticIntMethod(env, g_libautodiag, mid_engine_rpm);
        bool mil_status   = (*env)->CallStaticBooleanMethod(env, g_libautodiag, mid_mil_status);
        bool dtc_cleared   = (*env)->CallStaticBooleanMethod(env, g_libautodiag, mid_dtc_cleared);

        jstring ecu_name_j = (*env)->CallStaticObjectMethod(env, g_libautodiag, mid_ecu_name);
        jstring vin_j = (*env)->CallStaticObjectMethod(env, g_libautodiag, mid_vin);

        const char *ecu_name = (*env)->GetStringUTFChars(env, ecu_name_j, 0);
        const char *vin = (*env)->GetStringUTFChars(env, vin_j, 0);

        jobjectArray dtcs = (jobjectArray)(*env)->CallStaticObjectMethod(env, g_libautodiag, mid_dtcs);

        jsize dtc_count = (*env)->GetArrayLength(env, dtcs);

        final Buffer *binResponse = buffer_new();
        sim_ecu_generator_fill_success(binResponse, binRequest);
        switch(binRequest->buffer[0]) {
            case OBD_SERVICE_CLEAR_DTC: {
                (*env)->CallStaticVoidMethod(
                    env,
                    g_libautodiag,
                    mid_set_dtc_cleared,
                    JNI_TRUE
                );
                log_msg(LOG_DEBUG, "Clearing DTCs");
            } break;
            case OBD_SERVICE_SHOW_DTC: {
                if ( ! dtc_cleared ) {
                    for (jsize i = 0; i < dtc_count; i++) {
                        jstring s = (jstring)(*env)->GetObjectArrayElement(env, dtcs, i);
                        const char *dtc = (*env)->GetStringUTFChars(env, s, 0);

                        Buffer *dtc_bin = saej1979_dtc_bin_from_string((char*)dtc);
                        if ( dtc_bin == null ) {
                            log_msg(LOG_ERROR, "invalid dtc found");
                        } else {
                            buffer_append(binResponse, dtc_bin);
                        }
                        
                        (*env)->ReleaseStringUTFChars(env, s, dtc);
                        (*env)->DeleteLocalRef(env, s);
                    }
                }
            } break;
            case OBD_SERVICE_SHOW_CURRENT_DATA: {
                if ( 1 < binRequest->size ) {            
                    switch(binRequest->buffer[1]) {
                        case 0xC0:
                        case 0xA0:
                        case 0x80:
                        case 0x60:
                        case 0x40:
                        case 0x20:
                        case 0x00: {
                            buffer_append(binResponse, buffer_from_ascii_hex("FFFFFFFFFF"));
                        } break;
                        case 0x01: {
                            bool is_checked = mil_status;
                            Buffer* status = buffer_new();
                            buffer_padding(status, 4, 0x00);
                            if ( ! dtc_cleared ) {
                                status->buffer[0] = dtc_count;
                                status->buffer[0] |= is_checked << 7;
                            }
                            buffer_append(binResponse, status);
                        } break;
                        case 0x05: {
                            int coolant_coolant_temperature_abs = coolant_temperature - SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_MIN;
                            byte span = SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_MAX - SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_MIN;
                            double percent = (1.0 * coolant_coolant_temperature_abs) / span;
                            int value = percent * span;
                            buffer_append_byte(binResponse, (byte)(value));
                        } break;
                        case 0x0C: {
                            int engine_rpm_abs = engine_rpm - SAEJ1979_DATA_ENGINE_SPEED_MIN;
                            double span = SAEJ1979_DATA_ENGINE_SPEED_MAX - SAEJ1979_DATA_ENGINE_SPEED_MIN;
                            double percent = (1.0 * engine_rpm_abs) / span;
                            int value = percent * span * 4; // span * percent = (256 * A + B ) / 4
                            byte bA = (0xFF00 & value) >> 8;
                            byte bB = 0xFF & value;
                            buffer_append_byte(binResponse, bA);
                            buffer_append_byte(binResponse, bB);
                        } break;
                        case 0x0D: {
                            int vehicle_speed_abs = vehicle_speed - SAEJ1979_DATA_VEHICLE_SPEED_MIN;
                            byte span = SAEJ1979_DATA_VEHICLE_SPEED_MAX - SAEJ1979_DATA_VEHICLE_SPEED_MIN;
                            double percent = (1.0 * vehicle_speed_abs) / span;
                            int value = percent * span;
                            buffer_append_byte(binResponse, (byte)value);
                        } break;
                    }
                }
            } break;
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
                            final Buffer * name = buffer_from_ascii(ecu_name);
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
        (*env)->ReleaseStringUTFChars(env, ecu_name_j, ecu_name);
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