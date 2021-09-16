#include <jni.h>
#include <string>
#include "RealPlayer.h"
#include "JNICallbackHelper.h"
#include <android/native_window_jni.h>

RealPlayer *player;
JavaVM *vm;
ANativeWindow *window = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

jint JNI_OnLoad(JavaVM *pVm, void *args) {
    ::vm = pVm;
    return JNI_VERSION_1_6;
}

void renderCallback(uint8_t *src_data, int width, int height, int src_lineSize) {
    pthread_mutex_lock(&mutex);
    if (!window) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer window_buffer;

    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }
    // 渲染工作，填充 Buffer
    auto *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int dst_lineSize = window_buffer.stride * 4;

    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst_data + i * dst_lineSize,
               src_data + i * src_lineSize,
               dst_lineSize);
    }

    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mamba_player_Player_nativePrepare(JNIEnv *env, jobject thiz, jstring data_source_) {
    const char *data_source = env->GetStringUTFChars(data_source_, 0);
    auto *helper = new JNICallbackHelper(vm, env, thiz);
    // 创建播放器
    player = new RealPlayer(data_source, helper);
    __android_log_print(ANDROID_LOG_ERROR, "X_TAG", "Prepare");
    player->setRenderCallback(renderCallback);
    player->prepare();
    env->ReleaseStringUTFChars(data_source_, data_source);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mamba_player_Player_nativePlay(JNIEnv *env, jobject thiz) {
    if (player) {
        player->play();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mamba_player_Player_nativeStop(JNIEnv *env, jobject thiz) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_mamba_player_Player_nativeRelease(JNIEnv *env, jobject thiz) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_mamba_player_Player_nativeSetSurface(JNIEnv *env, jobject thiz, jobject surface) {
    pthread_mutex_lock(&mutex);

    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    window = ANativeWindow_fromSurface(env, surface);

    pthread_mutex_unlock(&mutex);
}