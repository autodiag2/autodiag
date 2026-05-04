#ifndef __AD_JNI_SIM_ELM327_H
#define __AD_JNI_SIM_ELM327_H

#include "libautodiag/compile_target.h"
#include "libautodiag/jni/loader.h"
#include "libautodiag/sim/elm327/elm327.h"

#ifdef OS_ANDROID
#include <jni.h>
#include <string.h>

SimELM327* jni_sim_elm327_get();
/**
 * @param path to a writable location on device
 * @param kind socket, network (for wifi server)
 */
JNIEXPORT jstring JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_launchEmu(JNIEnv *env, jobject thiz, jstring path, jstring kind);
JNIEXPORT void JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_setProtocol(JNIEnv *env, jobject thiz, jint protocol);
JNIEXPORT jobjectArray JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_getProtocols(JNIEnv *env, jobject thiz);
JNIEXPORT jint JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_getProtocol(JNIEnv *env, jobject thiz);
JNIEXPORT void JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_setResponseByteArrayByAddress(JNIEnv *env, jobject thiz, jbyte address, jobject callbackObject);
JNIEXPORT void JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_setResponseGuiByAddress(JNIEnv *env,jobject thiz,jbyte address);
JNIEXPORT void JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_removeEcuByAddress(JNIEnv *env, jobject thiz, jbyte address);
JNIEXPORT void JNICALL Java_com_github_autodiag2_elm327emu_libautodiag_setResponseTypeContextByAddress(JNIEnv *env,jobject thiz,jbyte address,jstring type,jstring context);
#endif

#endif