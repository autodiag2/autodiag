#include "libautodiag/jni/sim/ecu/generator/byte_array.h"

#ifdef OS_ANDROID
    static Buffer *response(SimECUGenerator *generator, Buffer *binRequest) {

        JavaCallbackContext *ctx = (JavaCallbackContext *)generator->context;

        JNIEnv *env;
        (*ctx->vm)->AttachCurrentThread(ctx->vm, (void **)&env, NULL);

        jbyteArray requestArray = (*env)->NewByteArray(env, binRequest->size);
        (*env)->SetByteArrayRegion(env, requestArray, 0,
            binRequest->size,
            (jbyte *)binRequest->buffer
        );

        jbyteArray result = (jbyteArray)(*env)->CallObjectMethod(
            env,
            ctx->callback,
            ctx->method,
            requestArray
        );

        // convert result back to Buffer
        jsize len = (*env)->GetArrayLength(env, result);
        jbyte *bytes = (*env)->GetByteArrayElements(env, result, NULL);

        Buffer *out = ad_buffer_from_bytes((byte*)bytes, len);

        (*env)->ReleaseByteArrayElements(env, result, bytes, JNI_ABORT);

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

        ctx->vm = getJavaVM(env); // store JVM reference
        ctx->callback = (*env)->NewGlobalRef(env, callbackObject);

        jclass cls = (*env)->GetObjectClass(env, callbackObject);
        ctx->method = (*env)->GetMethodID(
            env,
            cls,
            "response",
            "([B)[B"
        );
        generator->type = strdup("ByteArray");
        generator->context = ctx;
        generator->context_load_from_string = SIM_ECU_GENERATOR_CONTEXT_LOAD_FROM_STRING(context_load_from_string);
        generator->context_to_string = SIM_ECU_GENERATOR_CONTEXT_TO_STRING(context_to_string);
        generator->response = SIM_ECU_GENERATOR_RESPONSE(response);
        generator->flavour.is_Iso15765_4 = false;
        generator->response_saej1979_pid = null;
        generator->response_saej1979_dtcs = null;
        generator->response_saej1979_vehicle_identification_request_info_type = null;
        generator->state = null;
        return generator;
    }
#endif