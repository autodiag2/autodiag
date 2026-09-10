#include "libautodiag/jni/sim/ecu/generator/byte_array.h"

#ifdef OS_ANDROID
    static Buffer *response(SimECUGenerator *generator, Buffer *binRequest) {

        JavaCallbackContext *ctx = (JavaCallbackContext *)generator->context;

        JNIEnv *env = get_env();
        if (ctx == null || ctx->callback == null || ctx->method == null || env == null) {
            return ad_buffer_new();
        }

        jbyteArray requestArray = (*env)->NewByteArray(env, binRequest->size);
        if (requestArray == null || (*env)->ExceptionCheck(env)) {
            (*env)->ExceptionClear(env);
            return ad_buffer_new();
        }
        (*env)->SetByteArrayRegion(env, requestArray, 0,
            binRequest->size,
            (jbyte *)binRequest->buffer
        );
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionClear(env);
            (*env)->DeleteLocalRef(env, requestArray);
            return ad_buffer_new();
        }

        jbyteArray result = (jbyteArray)(*env)->CallObjectMethod(
            env,
            ctx->callback,
            ctx->method,
            requestArray
        );

        if ((*env)->ExceptionCheck(env) || result == null) {
            (*env)->ExceptionClear(env);
            (*env)->DeleteLocalRef(env, requestArray);
            if (result != null) {
                (*env)->DeleteLocalRef(env, result);
            }
            return ad_buffer_new();
        }

        // convert result back to Buffer
        jsize len = (*env)->GetArrayLength(env, result);
        jbyte *bytes = (*env)->GetByteArrayElements(env, result, NULL);

        if ((*env)->ExceptionCheck(env) || bytes == null) {
            (*env)->ExceptionClear(env);
            (*env)->DeleteLocalRef(env, requestArray);
            (*env)->DeleteLocalRef(env, result);
            return ad_buffer_new();
        }

        Buffer *out = ad_buffer_from_bytes((byte*)bytes, len);

        (*env)->ReleaseByteArrayElements(env, result, bytes, JNI_ABORT);
        (*env)->DeleteLocalRef(env, requestArray);
        (*env)->DeleteLocalRef(env, result);
        return out;
    }
    static char * context_to_string(SimECUGenerator * this) {
        return strdup("");
    }
    static bool context_load_from_string(SimECUGenerator * this, char * context) {
        return true;
    }
    SimECUGenerator* sim_ecu_generator_new_byte_array(
        jobject callbackObject
    ) {
        JNIEnv *env = get_env();
        SimECUGenerator *generator = sim_ecu_generator_new();

        JavaCallbackContext *ctx = malloc(sizeof(JavaCallbackContext));

        ctx->vm = getJavaVM(); // store JVM reference
        ctx->callback = (*env)->NewGlobalRef(env, callbackObject);

        jclass cls = (*env)->GetObjectClass(env, callbackObject);
        ctx->method = (*env)->GetMethodID(
            env,
            cls,
            "response",
            "([B)[B"
        );
        generator->type = strdup(SIM_ECU_GENERATOR_TYPE_ARRAY);
        generator->context = ctx;
        generator->context_load_from_string = SIM_ECU_GENERATOR_CONTEXT_LOAD_FROM_STRING(context_load_from_string);
        generator->context_to_string = SIM_ECU_GENERATOR_CONTEXT_TO_STRING(context_to_string);
        generator->response = SIM_ECU_GENERATOR_RESPONSE(response);
        generator->flavour.is_Iso15765_4 = false;
        generator->response_saej1979_pid = null;
        generator->response_saej1979_dtcs = null;
        generator->response_saej1979_vehicle_identification_request_info_type = null;
        generator->state = null;
        (*env)->DeleteLocalRef(env, cls);
        return generator;
    }
#endif