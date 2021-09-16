#include <jni.h>
#include <string>
#include "RealPlayer.h"
#include "JNICallbackHelper.h"

RealPlayer *player;
JavaVM *vm;

jint JNI_OnLoad(JavaVM *pVm, void *args) {
    ::vm = pVm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mamba_player_Player_nativePrepare(JNIEnv *env, jobject thiz, jstring data_source_) {
    const char *data_source = env->GetStringUTFChars(data_source_, 0);
    auto *helper = new JNICallbackHelper(vm, env, thiz);
    // 创建播放器
    player = new RealPlayer(data_source, helper);
    player->prepare();
    env->ReleaseStringUTFChars(data_source_, data_source);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mamba_player_Player_nativePlay(JNIEnv *env, jobject thiz) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_mamba_player_Player_nativeStop(JNIEnv *env, jobject thiz) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_mamba_player_Player_nativeRelease(JNIEnv *env, jobject thiz) {

}